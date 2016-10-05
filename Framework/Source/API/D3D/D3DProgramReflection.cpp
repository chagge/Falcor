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
#include "Framework.h"
#ifdef FALCOR_D3D
#include "API/ProgramReflection.h"
#include "API/ProgramVersion.h"
#include "API/Shader.h"

namespace Falcor
{
#ifdef FALCOR_D3D11
#elif defined FALCOR_D3D12
    using D3D_SHADER_DESC = D3D12_SHADER_DESC;
    using D3D_SHADER_BUFFER_DESC = D3D12_SHADER_BUFFER_DESC;
    using ID3DShaderReflectionConstantBuffer = ID3D12ShaderReflectionConstantBuffer;
    using ID3DShaderReflectionVariable = ID3D12ShaderReflectionVariable;
    using ID3DShaderReflectionType = ID3D12ShaderReflectionType;
    using D3D_SHADER_TYPE_DESC = D3D12_SHADER_TYPE_DESC;
    using D3D_SHADER_VARIABLE_DESC = D3D12_SHADER_VARIABLE_DESC;
    using D3D_SHADER_INPUT_BIND_DESC = D3D12_SHADER_INPUT_BIND_DESC;
    using D3D_SIGNATURE_PARAMETER_DESC = D3D12_SIGNATURE_PARAMETER_DESC;

#endif

    ProgramReflection::Variable::Type getVariableType(D3D_SHADER_VARIABLE_TYPE dxType, uint32_t rows, uint32_t columns)
    {
        if (dxType == D3D_SVT_BOOL)
        {
            assert(rows == 1);
            switch (columns)
            {
            case 1:
                return ProgramReflection::Variable::Type::Bool;
            case 2:
                return ProgramReflection::Variable::Type::Bool2;
            case 3:
                return ProgramReflection::Variable::Type::Bool3;
            case 4:
                return ProgramReflection::Variable::Type::Bool4;
            }
        }
        else if (dxType == D3D_SVT_UINT)
        {
            assert(rows == 1);
            switch (columns)
            {
            case 1:
                return ProgramReflection::Variable::Type::Uint;
            case 2:
                return ProgramReflection::Variable::Type::Uint2;
            case 3:
                return ProgramReflection::Variable::Type::Uint3;
            case 4:
                return ProgramReflection::Variable::Type::Uint4;
            }
        }
        else if (dxType == D3D_SVT_INT)
        {
            assert(rows == 1);
            switch (columns)
            {
            case 1:
                return ProgramReflection::Variable::Type::Int;
            case 2:
                return ProgramReflection::Variable::Type::Int2;
            case 3:
                return ProgramReflection::Variable::Type::Int3;
            case 4:
                return ProgramReflection::Variable::Type::Int4;
            }
        }
        else if (dxType == D3D_SVT_FLOAT)
        {
            switch (rows)
            {
            case 1:
                switch (columns)
                {
                case 1:
                    return ProgramReflection::Variable::Type::Float;
                case 2:
                    return ProgramReflection::Variable::Type::Float2;
                case 3:
                    return ProgramReflection::Variable::Type::Float3;
                case 4:
                    return ProgramReflection::Variable::Type::Float4;
                }
                break;
            case 2:
                switch (columns)
                {
                case 2:
                    return ProgramReflection::Variable::Type::Float2x2;
                case 3:
                    return ProgramReflection::Variable::Type::Float2x3;
                case 4:
                    return ProgramReflection::Variable::Type::Float2x4;
                }
                break;
            case 3:
                switch (columns)
                {
                case 2:
                    return ProgramReflection::Variable::Type::Float3x2;
                case 3:
                    return ProgramReflection::Variable::Type::Float3x3;
                case 4:
                    return ProgramReflection::Variable::Type::Float3x4;
                }
                break;
            case 4:
                switch (columns)
                {
                case 2:
                    return ProgramReflection::Variable::Type::Float4x2;
                case 3:
                    return ProgramReflection::Variable::Type::Float4x3;
                case 4:
                    return ProgramReflection::Variable::Type::Float4x4;
                }
                break;
            }
        }

        should_not_get_here();
        return ProgramReflection::Variable::Type::Unknown;
    }

    size_t getRowCountFromType(ProgramReflection::Variable::Type type)
    {
        switch (type)
        {
        case ProgramReflection::Variable::Type::Unknown:
            return 0;
        case ProgramReflection::Variable::Type::Bool:
        case ProgramReflection::Variable::Type::Bool2:
        case ProgramReflection::Variable::Type::Bool3:
        case ProgramReflection::Variable::Type::Bool4:
        case ProgramReflection::Variable::Type::Uint:
        case ProgramReflection::Variable::Type::Uint2:
        case ProgramReflection::Variable::Type::Uint3:
        case ProgramReflection::Variable::Type::Uint4:
        case ProgramReflection::Variable::Type::Int:
        case ProgramReflection::Variable::Type::Int2:
        case ProgramReflection::Variable::Type::Int3:
        case ProgramReflection::Variable::Type::Int4:
        case ProgramReflection::Variable::Type::Float:
        case ProgramReflection::Variable::Type::Float2:
        case ProgramReflection::Variable::Type::Float3:
        case ProgramReflection::Variable::Type::Float4:
            return 1;
        case ProgramReflection::Variable::Type::Float2x2:
        case ProgramReflection::Variable::Type::Float2x3:
        case ProgramReflection::Variable::Type::Float2x4:
            return 2;
        case ProgramReflection::Variable::Type::Float3x2:
        case ProgramReflection::Variable::Type::Float3x3:
        case ProgramReflection::Variable::Type::Float3x4:
            return 3;
        case ProgramReflection::Variable::Type::Float4x2:
        case ProgramReflection::Variable::Type::Float4x3:
        case ProgramReflection::Variable::Type::Float4x4:
            return 4;
        default:
            should_not_get_here();
            return 0;
        }
    }

    size_t calcStructSize(const ProgramReflection::VariableMap& varMap)
    {
        size_t maxOffset = 0;
        ProgramReflection::Variable::Type type = varMap.begin()->second.type;

        for (const auto& a : varMap)
        {
            if (a.second.location > maxOffset)
            {
                maxOffset = a.second.location;
                type = a.second.type;
            }
        }
        if (maxOffset > 0)
        {
            maxOffset += getRowCountFromType(type) * 16;
        }
        return maxOffset;
    }

    static void reflectType(ID3DShaderReflectionType* pType, ProgramReflection::VariableMap& varMap, const std::string& name, size_t offset)
    {
        D3D_SHADER_TYPE_DESC typeDesc;
        pType->GetDesc(&typeDesc);

        offset += typeDesc.Offset;

        if (typeDesc.Class == D3D_SVC_STRUCT)
        {
            // Calculate the structure size
            ProgramReflection::VariableMap structVarMap;
            for (uint32_t memberID = 0; memberID < typeDesc.Members; memberID++)
            {
                ID3DShaderReflectionType* pMember = pType->GetMemberTypeByIndex(memberID);
                std::string memberName(pType->GetMemberTypeName(memberID));
                reflectType(pMember, structVarMap, memberName, 0);
            }
            size_t structSize = calcStructSize(structVarMap);

            // Parse the internal variables
            for (uint32_t memberID = 0; memberID < typeDesc.Members; memberID++)
            {
                ID3DShaderReflectionType* pMember = pType->GetMemberTypeByIndex(memberID);
                std::string memberName(pType->GetMemberTypeName(memberID));

                if (typeDesc.Elements > 0)
                {
                    for (uint32_t i = 0; i < typeDesc.Elements; i++)
                    {
                        reflectType(pMember, varMap, name + '[' + std::to_string(i) + "]." + memberName, offset + structSize * i);
                    }
                }
                else
                {
                    reflectType(pMember, varMap, name + "." + memberName, offset);
                }
            }
        }
        else
        {
            ProgramReflection::Variable desc;
            desc.arraySize = typeDesc.Elements;
            desc.isRowMajor = (typeDesc.Class == D3D_SVC_MATRIX_ROWS);
            desc.location = offset;
            desc.type = getVariableType(typeDesc.Type, typeDesc.Rows, typeDesc.Columns);
            varMap[name] = desc;
        }
    }

    static void reflectVariable(ID3DShaderReflectionConstantBuffer* pReflector, ID3D12ShaderReflectionVariable* pVar, ProgramReflection::VariableMap& varMap)
    {
        // Get the variable name
        D3D_SHADER_VARIABLE_DESC varDesc;
        pVar->GetDesc(&varDesc);
        std::string name(varDesc.Name);

        // Reflect the Type
        ID3DShaderReflectionType* pType = pVar->GetType();
        reflectType(pType, varMap, name, varDesc.StartOffset);
    }

    static void initializeBufferVariables(ID3DShaderReflectionConstantBuffer* pReflector, const D3D_SHADER_BUFFER_DESC& desc, ProgramReflection::VariableMap& varMap)
    {
        for (uint32_t varID = 0; varID < desc.Variables; varID++)
        {
            ID3DShaderReflectionVariable* pVar = pReflector->GetVariableByIndex(varID);
            reflectVariable(pReflector, pVar, varMap);
        }

        assert(calcStructSize(varMap) == desc.Size);
    }

    bool validateBufferDeclaration(const ProgramReflection::BufferReflection* pPrevDesc, const ProgramReflection::VariableMap& varMap, std::string& log)
    {
        bool match = true;
#define error_msg(msg_) std::string(msg_) + " mismatch.\n";
        if (pPrevDesc->getVariableCount() != varMap.size())
        {
            log += error_msg("Variable count");
            match = false;
        }

        for (auto& prevVar = pPrevDesc->varBegin(); prevVar != pPrevDesc->varEnd(); prevVar++)
        {
            const std::string& name = prevVar->first;
            const auto& curVar = varMap.find(name);
            if (curVar == varMap.end())
            {
                log += "Can't find variable '" + name + "' in the new definitions";
            }
            else
            {
#define test_field(field_, msg_)                                      \
            if(prevVar->second.field_ != curVar->second.field_)       \
            {                                                         \
                log += error_msg(prevVar->first + " " + msg_)         \
                match = false;                                       \
            }

                test_field(location, "offset");
                test_field(arraySize, "array size");
                test_field(arrayStride, "array stride");
                test_field(isRowMajor, "row major");
                test_field(type, "Type");
#undef test_field
            }
        }

#undef error_msg

        return match;
    }

    bool ProgramReflection::reflectBuffers(const ProgramVersion* pProgVer, std::string& log)
    {
        for (uint32_t shader = 0; shader < (uint32_t)ShaderType::Count; shader++)
        {
            ShaderReflectionHandle pReflection = pProgVer->getShader((ShaderType)shader) ? pProgVer->getShader(ShaderType(shader))->getReflectionInterface() : nullptr;
            if (pReflection)
            {
                // Find all the buffers
                D3D_SHADER_DESC shaderDesc;
                d3d_call(pReflection->GetDesc(&shaderDesc));

                // CBs doesn't have to be continuous, we loop until we find all available buffers
                for (uint32_t found = 0, cbIndex = 0; found < shaderDesc.ConstantBuffers; cbIndex++)
                {
                    D3D_SHADER_BUFFER_DESC d3dBufDesc;
                    ID3DShaderReflectionConstantBuffer* pBuffer = pReflection->GetConstantBufferByIndex(cbIndex);
                    d3d_call(pBuffer->GetDesc(&d3dBufDesc));

                    if ((d3dBufDesc.Type == D3D_CT_TBUFFER) || (d3dBufDesc.Type == D3D_CT_CBUFFER))
                    {
                        D3D_SHADER_INPUT_BIND_DESC bindDesc;
                        d3d_call(pReflection->GetResourceBindingDescByName(d3dBufDesc.Name, &bindDesc));
                        assert(bindDesc.BindCount == 1);
                        assert(d3dBufDesc.Type == D3D_CT_CBUFFER);  // Not sure how to handle texture buffers
                        BufferReflection::Type bufferType = (d3dBufDesc.Type == D3D_CT_CBUFFER) ? BufferReflection::Type::Constant : BufferReflection::Type::UnorderedAccess;
                        ProgramReflection::VariableMap varMap;
                        initializeBufferVariables(pBuffer, d3dBufDesc, varMap);

                        // If the buffer already exists in the program, make sure the definitions match
                        BufferData& bufferDesc = mBuffers[(uint32_t)bufferType];
                        const auto& prevDef = bufferDesc.nameMap.find(d3dBufDesc.Name);
                        if (prevDef != bufferDesc.nameMap.end())
                        {
                            if (bindDesc.BindPoint != prevDef->second)
                            {
                                log += "Constant buffer '" + std::string(d3dBufDesc.Name) + "' has different bind locations between different shader stages. Falcor do not support that. Use explicit bind locations to avoid this error";
                                return false;
                            }
                            ProgramReflection::BufferReflection* pPrevBuffer = bufferDesc.descMap[bindDesc.BindPoint].get();
                            std::string bufLog;
                            if (validateBufferDeclaration(pPrevBuffer, varMap, bufLog) == false)
                            {
                                log += "Constant buffer '" + std::string(d3dBufDesc.Name) + "' has different definitions between different shader stages. " + bufLog;
                                return false;
                            }
                        }
                        else
                        {
                            // Create the buffer reflection
                            bufferDesc.nameMap[d3dBufDesc.Name] = bindDesc.BindPoint;
                            bufferDesc.descMap[bindDesc.BindPoint] = ProgramReflection::BufferReflection::create(d3dBufDesc.Name, bindDesc.BindPoint, BufferReflection::Type::Constant, d3dBufDesc.Size, d3dBufDesc.Variables, varMap, ProgramReflection::ResourceMap());
                        }

                        // Update the shader mask
                        uint32_t mask = bufferDesc.descMap[bindDesc.BindPoint]->getShaderMask() | (1 << shader);
                        bufferDesc.descMap[bindDesc.BindPoint]->setShaderMask(mask);
                        found++;
                    }
                }
            }
        }
        return true;
    }

    bool ProgramReflection::reflectVertexAttributes(const ProgramVersion* pProgVer, std::string& log)
    {
        // Get the VS. We must have one
        ShaderReflectionHandle pReflector = pProgVer->getShader(ShaderType::Vertex)->getReflectionInterface();
        assert(pReflector);
        D3D_SHADER_DESC shaderDesc;
        d3d_call(pReflector->GetDesc(&shaderDesc));
        for (uint32_t i = 0; i < shaderDesc.InputParameters; i++)
        {
            D3D_SIGNATURE_PARAMETER_DESC inputDesc;
            d3d_call(pReflector->GetInputParameterDesc(i, &inputDesc));
        }
        return true;
    }

    bool ProgramReflection::reflectFragmentOutputs(const ProgramVersion* pProgVer, std::string& log)
    {
        return true;
    }

    static bool verifyResourceDefinition(const ProgramReflection::Resource& prev, ProgramReflection::Resource& current, std::string& log)
    {
        bool match = true;
#define error_msg(msg_) std::string(msg_) + " mismatch.\n";
#define test_field(field_)                                           \
            if(prev.field_ != current.field_)                        \
            {                                                        \
                log += error_msg(#field_)                            \
                match = false;                                       \
            }

        test_field(type);
        test_field(dims);
        test_field(retType);
        test_field(regIndex);
        test_field(registerSpace);
        test_field(arraySize);
#undef test_field
#undef error_msg

        return match;
    }

    static ProgramReflection::Resource::Dimensions getResourceDimensions(D3D_SRV_DIMENSION dims)
    {
        switch (dims)
        {
        case D3D_SRV_DIMENSION_BUFFER:
            return ProgramReflection::Resource::Dimensions::TextureBuffer;
        case D3D_SRV_DIMENSION_TEXTURE1D:
            return ProgramReflection::Resource::Dimensions::Texture1D;
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
            return ProgramReflection::Resource::Dimensions::Texture1DArray;
        case D3D_SRV_DIMENSION_TEXTURE2D:
            return ProgramReflection::Resource::Dimensions::Texture2D;
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
            return ProgramReflection::Resource::Dimensions::Texture2DArray;
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
            return ProgramReflection::Resource::Dimensions::Texture2DMS;
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
            return ProgramReflection::Resource::Dimensions::Texture2DMSArray;
        case D3D_SRV_DIMENSION_TEXTURE3D:
            return ProgramReflection::Resource::Dimensions::Texture3D;
        case D3D_SRV_DIMENSION_TEXTURECUBE:
            return ProgramReflection::Resource::Dimensions::TextureCube;
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            return ProgramReflection::Resource::Dimensions::TextureCubeArray;
        default:
            should_not_get_here();
            return ProgramReflection::Resource::Dimensions::Unknown;
        }
    }

    static ProgramReflection::Resource::ResourceType getResourceType(D3D_SHADER_INPUT_TYPE type)
    {
        switch (type)
        {
        case D3D_SIT_TEXTURE:
            return ProgramReflection::Resource::ResourceType::Texture;
        case D3D_SIT_SAMPLER:
            return ProgramReflection::Resource::ResourceType::Sampler;
        default:
            should_not_get_here();
            return ProgramReflection::Resource::ResourceType::Unknown;
        }
    }

    static ProgramReflection::Resource::ReturnType getReturnType(D3D_RESOURCE_RETURN_TYPE type)
    {
        switch (type)
        {
        case D3D_RETURN_TYPE_UNORM:
        case D3D_RETURN_TYPE_SNORM:
        case D3D_RETURN_TYPE_FLOAT:
            return ProgramReflection::Resource::ReturnType::Float;
        case D3D_RETURN_TYPE_SINT:
            return ProgramReflection::Resource::ReturnType::Int;
        case D3D_RETURN_TYPE_UINT:
            return ProgramReflection::Resource::ReturnType::Uint;
        case D3D_RETURN_TYPE_DOUBLE:
            return ProgramReflection::Resource::ReturnType::Double;
        default:
            should_not_get_here();
            return ProgramReflection::Resource::ReturnType::Unknown;
        }
    }

    bool ProgramReflection::reflectResources(const ProgramVersion* pProgVer, std::string& log)
    {
        for (uint32_t shader = 0; shader < (uint32_t)ShaderType::Count; shader++)
        {
            ShaderReflectionHandle pReflection = pProgVer->getShader((ShaderType)shader) ? pProgVer->getShader(ShaderType(shader))->getReflectionInterface() : nullptr;
            if (pReflection)
            {
                D3D_SHADER_DESC shaderDesc;
                d3d_call(pReflection->GetDesc(&shaderDesc));

                for (uint32_t i = 0; i < shaderDesc.BoundResources; i++)
                {
                    D3D_SHADER_INPUT_BIND_DESC InputDesc;
                    d3d_call(pReflection->GetResourceBindingDesc(i, &InputDesc));

                    // Ignore constant buffers
                    if (InputDesc.Type == D3D_SIT_CBUFFER)
                    {
                        continue;
                    }
                    ProgramReflection::Resource falcorDesc;
                    std::string name(InputDesc.Name);
                    falcorDesc.type = getResourceType(InputDesc.Type);
                    if (falcorDesc.type != ProgramReflection::Resource::ResourceType::Sampler)
                    {
                        falcorDesc.retType = getReturnType(InputDesc.ReturnType);
                        falcorDesc.dims = getResourceDimensions(InputDesc.Dimension);
                    }
                    bool isArray = name[name.length() - 1] == ']';
                    falcorDesc.regIndex = InputDesc.BindPoint;
                    falcorDesc.arraySize = isArray ? InputDesc.BindCount : 0;

                    // If this already exists, definitions should match
                    const auto& prevDef = mResources.find(name);
                    if (prevDef == mResources.end())
                    {
                        // New resource
                        mResources[name] = falcorDesc;
                    }
                    else
                    {
                        std::string varLog;
                        if (verifyResourceDefinition(prevDef->second, falcorDesc, varLog) == false)
                        {
                            log += "Shader resource '" + std::string(name) + "' has different definitions between different shader stages. " + varLog;
                            return false;
                        }
                    }
                    // Update the mask
                    mResources[name].shaderMask |= (1 << shader);
                }
            }
        }
        return true;
    }
}
#endif //#ifdef FALCOR_D3D
