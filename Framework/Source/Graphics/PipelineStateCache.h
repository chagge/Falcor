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
#pragma once
#include "API/PipelineState.h"
#include "Graphics/Program.h"
#include "API/VAO.h"
#include "API/FBO.h"
#include "API/RasterizerState.h"
#include "API/DepthStencilState.h"
#include "API/BlendState.h"

namespace Falcor
{
    class PipelineStateCache
    {
    public:
        using SharedPtr = std::shared_ptr<PipelineStateCache>;
        using SharedConstPtr = std::shared_ptr<const PipelineStateCache>;
        
        static SharedPtr create() { return SharedPtr(new PipelineStateCache); }
        PipelineStateCache& SetVao(const Vao::SharedPtr& pVao) { mpVao = pVao; return *this; }
        PipelineStateCache& setFbo(const Fbo::SharedPtr& pFbo) { mpFbo = pFbo; return *this; }
        PipelineStateCache& setProgram(const Program::SharedPtr& pProgram) { mpProgram = pProgram; return *this; }
        PipelineStateCache& setBlendState(BlendState::SharedPtr pBlendState) { mDesc.setBlendState(pBlendState); return *this; }
        PipelineStateCache& setRasterizerState(RasterizerState::SharedPtr pRasterizerState) { mDesc.setRasterizerState(pRasterizerState); return *this; }
        PipelineStateCache& setDepthStencilState(DepthStencilState::SharedPtr pDepthStencilState) { mDesc.setDepthStencilState(pDepthStencilState); return *this; }
        PipelineStateCache& setSampleMask(uint32_t sampleMask) { mDesc.setSampleMask(sampleMask); return *this; }
        PipelineStateCache& setPrimitiveType(PipelineState::PrimitiveType type) { mDesc.setPrimitiveType(type); return *this; }
        PipelineStateCache& setRootSignature(RootSignature::SharedPtr pSignature) { mpRootSignature = pSignature; mCachedData.isUserRootSignature = (mpRootSignature == nullptr); }

        PipelineState::SharedPtr getPSO();

        RootSignature::SharedPtr getRootSignature() const { return mpRootSignature; }
        Vao::SharedPtr getVao() const { return mpVao; }
        Fbo::SharedPtr getFbo() const { return mpFbo; }
        Program::SharedPtr getProgram() const { return mpProgram; }
        BlendState::SharedPtr getBlendState() const { return mDesc.getBlendState(); }
        RasterizerState::SharedPtr getRasterizerState() const { return mDesc.getRasterizerState(); }
        DepthStencilState::SharedPtr getDepthStencilState() const { return mDesc.getDepthStencilState(); }
        uint32_t getSampleMask() const { return mDesc.getSampleMask(); }
        PipelineState::PrimitiveType getPrimitiveType() const { return mDesc.getPrimitiveType(); }
    private:
        PipelineStateCache() = default;
        Vao::SharedPtr mpVao;
        Fbo::SharedPtr mpFbo;
        Program::SharedPtr mpProgram;
        RootSignature::SharedPtr mpRootSignature;
        PipelineState::Desc mDesc;

        struct CachedData
        {
            const ProgramVersion* pProgramVersion = nullptr;
            bool isUserRootSignature = false;
        };
        CachedData mCachedData;
    };
}