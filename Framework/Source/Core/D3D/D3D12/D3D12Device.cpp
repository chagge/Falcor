/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#ifdef FALCOR_D3D12
#include "Framework.h"
#include "Sample.h"
#include "Core/Device.h"
#include "D3D12DescriptorHeap.h"
#include "D3D12Fence.h"

namespace Falcor
{
	struct DeviceData
	{
		IDXGISwapChain3Ptr pSwapChain = nullptr;
		uint32_t currentBackBufferIndex;
        Fbo::SharedPtr pDefaultFbos[kSwapChainBuffers];
        ID3D12CommandQueuePtr pCommandQueue;
        Fence::SharedPtr pFence;
        uint64_t frameIndex = 0;
		uint32_t syncInterval = 0;
		bool isWindowOccluded = false;
	};

	DeviceHandle Device::sApiHandle;
	void* Device::mpPrivateData = nullptr;

	void d3dTraceHR(const std::string& msg, HRESULT hr)
	{
		char hr_msg[512];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, hr, 0, hr_msg, ARRAYSIZE(hr_msg), nullptr);

		std::string error_msg = msg + ".\nError " + hr_msg;
		Logger::log(Logger::Level::Fatal, error_msg);
	}

    D3D_FEATURE_LEVEL getD3DFeatureLevel(uint32_t majorVersion, uint32_t minorVersion)
    {
        if(majorVersion == 12)
        {
            switch(minorVersion)
            {
            case 0:
                return D3D_FEATURE_LEVEL_12_0;
            case 1:
                return D3D_FEATURE_LEVEL_12_1;
            }
        }
        else if(majorVersion == 11)
        {
            switch(minorVersion)
            {
            case 0:
                return D3D_FEATURE_LEVEL_11_1;
            case 1:
                return D3D_FEATURE_LEVEL_11_0;
            }
        }
        else if(majorVersion == 10)
        {
            switch(minorVersion)
            {
            case 0:
                return D3D_FEATURE_LEVEL_10_0;
            case 1:
                return D3D_FEATURE_LEVEL_10_1;
            }
        }
        else if(majorVersion == 9)
        {
            switch(minorVersion)
            {
            case 1:
                return D3D_FEATURE_LEVEL_9_1;
            case 2:
                return D3D_FEATURE_LEVEL_9_2;
            case 3:
                return D3D_FEATURE_LEVEL_9_3;
            }
        }
        return (D3D_FEATURE_LEVEL)0;
    }

	IDXGISwapChain3Ptr createSwapChain(IDXGIFactory4* pFactory, const Window* pWindow, ID3D12CommandQueue* pCommandQueue, ResourceFormat colorFormat)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = kSwapChainBuffers;
		swapChainDesc.Width = pWindow->getClientAreaWidth();
		swapChainDesc.Height = pWindow->getClientAreaHeight();
		// Flip mode doesn't support SRGB formats, so we strip them down when creating the resource. We will create the RTV as SRGB instead.
		// More details at the end of https://msdn.microsoft.com/en-us/library/windows/desktop/bb173064.aspx
		swapChainDesc.Format = getDxgiFormat(srgbToLinearFormat(colorFormat));
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		// CreateSwapChainForHwnd() doesn't accept IDXGISwapChain3 (Why MS? Why?)
		MAKE_SMART_COM_PTR(IDXGISwapChain1);
		IDXGISwapChain1Ptr pSwapChain;

		HRESULT hr = pFactory->CreateSwapChainForHwnd(pCommandQueue, pWindow->getApiHandle(), &swapChainDesc, nullptr, nullptr, &pSwapChain);
		if (FAILED(hr))
		{
			d3dTraceHR("Failed to create the swap-chain", hr);
			return false;
		}

		IDXGISwapChain3Ptr pSwapChain3;
		d3d_call(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3)));
		return pSwapChain3;
	}

	ID3D12DevicePtr createDevice(IDXGIFactory4* pFactory, D3D_FEATURE_LEVEL featureLevel)
	{
		// Find the HW adapter
		IDXGIAdapter1Ptr pAdapter;
		ID3D12DevicePtr pDevice;

		for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(i, &pAdapter); i++)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);

			// Skip SW adapters
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			// Try and create a D3D12 device
			if (D3D12CreateDevice(pAdapter, featureLevel, IID_PPV_ARGS(&pDevice)) == S_OK)
			{
				return pDevice;
			}
		}

		Logger::log(Logger::Level::Fatal, "Could not find a GPU that supports D3D12 device");
		return nullptr;
	}

	bool Device::updateDefaultFBO(uint32_t width, uint32_t height, uint32_t sampleCount, ResourceFormat colorFormat, ResourceFormat depthFormat)
	{
        DeviceData* pData = (DeviceData*)mpPrivateData;

		for (uint32_t i = 0; i < kSwapChainBuffers; i++)
		{
            // Create a texture object
            auto pColorTex = Texture::SharedPtr(new Texture(width, height, 1, 1, 1, sampleCount, colorFormat, sampleCount > 1 ? Texture::Type::Texture2DMultisample : Texture::Type::Texture2D));            
            HRESULT hr = pData->pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pColorTex->mApiHandle));
            if(FAILED(hr))
            {
                d3dTraceHR("Failed to get back-buffer " + std::to_string(i) + " from the swap-chain", hr);
                return false;
            }

            // Create a depth texture
            auto pDepth = Texture::create2D(width, height, depthFormat, 1, 1);

            // Create the FBO
            pData->pDefaultFbos[i] = Fbo::create();
            pData->pDefaultFbos[i]->attachColorTarget(pColorTex, 0);
            pData->pDefaultFbos[i]->attachDepthStencilTarget(pDepth);
            pData->currentBackBufferIndex = pData->pSwapChain->GetCurrentBackBufferIndex();
		}

		return true;
	}

	Device::~Device() = default;

	Device::SharedPtr Device::create(Window::SharedPtr& pWindow, const Device::Desc& desc)
	{
		SharedPtr pDevice = SharedPtr(new Device(pWindow));
		return pDevice->init(desc) ? pDevice : nullptr;
	}

    
    Fbo::SharedPtr Device::getSwapChainFbo() const
    {
        DeviceData* pData = (DeviceData*)mpPrivateData;
        return pData->pDefaultFbos[pData->currentBackBufferIndex];
    }

	void Device::present()
	{
		DeviceData* pData = (DeviceData*)mpPrivateData;

		// Submit the command list
		auto pGfxList = mpRenderContext->getCommandListApiHandle();
		d3d_call(pGfxList->Close());
		ID3D12CommandList* pList = pGfxList.GetInterfacePtr();
		pData->pCommandQueue->ExecuteCommandLists(1, &pList);

		// Present
		pData->pSwapChain->Present(1, 0);
		pData->currentBackBufferIndex = (pData->currentBackBufferIndex + 1) % kSwapChainBuffers;

        pData->frameIndex++;
        pData->pFence->signal(pData->pCommandQueue, pData->frameIndex);

        // Wait until the selected back-buffer is ready
        if(pData->frameIndex > kSwapChainBuffers)
        {
            pData->pFence->wait(pData->frameIndex - kSwapChainBuffers);
        }
	}

	bool Device::init(const Desc& desc)
	{
		if (sApiHandle)
		{
			logError("D3D12 backend only supports a single device");
			return false;
		}

		DeviceData* pData = new DeviceData;
		mpPrivateData = pData;

#if defined(_DEBUG)
		{
			ID3D12DebugPtr pDebug;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug))))
			{
				pDebug->EnableDebugLayer();
			}
		}
#endif
		// Create the DXGI factory
		IDXGIFactory4Ptr pDxgiFactory;
		d3d_call(CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory)));

		// Create the device
		sApiHandle = createDevice(pDxgiFactory, getD3DFeatureLevel(desc.apiMajorVersion, desc.apiMinorVersion));
		if (sApiHandle == nullptr)
		{
			return false;
		}

		// Create the command queue
		D3D12_COMMAND_QUEUE_DESC cqDesc = {};
		cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ID3D12DevicePtr pDevice = Device::getApiHandle();

		if (FAILED(pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pData->pCommandQueue))))
		{
			Logger::log(Logger::Level::Error, "Failed to create command queue");
			return nullptr;
		}

		// Create the swap-chain
		pData->pSwapChain = createSwapChain(pDxgiFactory, mpWindow.get(), pData->pCommandQueue, desc.colorFormat);
		if(pData->pSwapChain == nullptr)
		{
			return false;
		}

        // Update the FBOs
        if(updateDefaultFBO(mpWindow->getClientAreaWidth(), mpWindow->getClientAreaHeight(), 1, desc.colorFormat, desc.depthFormat) == false)
        {
            return false;
        }

        mpRenderContext = RenderContext::create(kSwapChainBuffers);

		mVsyncOn = desc.enableVsync;

        pData->pFence = Fence::create();
		return true;
    }

	void Device::setVSync(bool enable)
	{
		DeviceData* pData = (DeviceData*)mpPrivateData;
		pData->syncInterval = enable ? 1 : 0;
	}

	bool Device::isWindowOccluded() const
    {
		DeviceData* pData = (DeviceData*)mpPrivateData;
        if(pData->isWindowOccluded)
        {
			pData->isWindowOccluded = (pData->pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED);
        }
        return pData->isWindowOccluded;
    }

    bool Device::isExtensionSupported(const std::string& name)
    {
        UNSUPPORTED_IN_D3D("Device::isExtensionSupported()");
        return false;
    }
}
#endif //#ifdef FALCOR_D3D
