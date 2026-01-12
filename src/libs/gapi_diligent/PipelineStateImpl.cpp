#include "PipelineStateImpl.hpp"

#include "RenderDevice.h"
#include "BlendState.h"
#include "PipelineState.h"

#include "gapi/PipelineState.hpp"
#include "gapi/Limits.hpp"

#include "gapi_diligent/ShaderImpl.hpp"
#include "gapi_diligent/Utils.hpp"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    DL::BLEND_OPERATION getBlendOp(GAPI::BlendOp op)
    {
        switch (op)
        {
            case GAPI::BlendOp::Add: return DL::BLEND_OPERATION_ADD;
            case GAPI::BlendOp::Subtract: return DL::BLEND_OPERATION_SUBTRACT;
            case GAPI::BlendOp::ReverseSubtract: return DL::BLEND_OPERATION_REV_SUBTRACT;
            case GAPI::BlendOp::Min: return DL::BLEND_OPERATION_MIN;
            case GAPI::BlendOp::Max: return DL::BLEND_OPERATION_MAX;
            default:
                ASSERT_MSG(false, "Unknown blend op");
                return DL::BLEND_OPERATION_UNDEFINED;
        }
    }

    DL::BLEND_FACTOR getBlendFunc(GAPI::BlendFactor func)
    {
        switch (func)
        {
            case GAPI::BlendFactor::Zero: return DL::BLEND_FACTOR_ZERO;
            case GAPI::BlendFactor::One: return DL::BLEND_FACTOR_ONE;
            case GAPI::BlendFactor::SrcColor: return DL::BLEND_FACTOR_SRC_COLOR;
            case GAPI::BlendFactor::OneMinusSrcColor: return DL::BLEND_FACTOR_INV_SRC_COLOR;
            case GAPI::BlendFactor::DstColor: return DL::BLEND_FACTOR_DEST_COLOR;
            case GAPI::BlendFactor::OneMinusDstColor: return DL::BLEND_FACTOR_INV_DEST_COLOR;
            case GAPI::BlendFactor::SrcAlpha: return DL::BLEND_FACTOR_SRC_ALPHA;
            case GAPI::BlendFactor::OneMinusSrcAlpha: return DL::BLEND_FACTOR_INV_SRC_ALPHA;
            case GAPI::BlendFactor::DstAlpha: return DL::BLEND_FACTOR_DEST_ALPHA;
            case GAPI::BlendFactor::OneMinusDstAlpha: return DL::BLEND_FACTOR_INV_DEST_ALPHA;
            case GAPI::BlendFactor::SrcAlphaSaturate: return DL::BLEND_FACTOR_SRC_ALPHA_SAT;
            case GAPI::BlendFactor::Src1Color: return DL::BLEND_FACTOR_SRC1_COLOR;
            case GAPI::BlendFactor::OneMinusSrc1Color: return DL::BLEND_FACTOR_INV_SRC1_COLOR;
            case GAPI::BlendFactor::Src1Alpha: return DL::BLEND_FACTOR_SRC1_ALPHA;
            case GAPI::BlendFactor::OneMinusSrc1Alpha: return DL::BLEND_FACTOR_INV_SRC1_ALPHA;
            default:
                ASSERT_MSG(false, "Unknown blend func");
                return DL::BLEND_FACTOR_UNDEFINED;
        }
    }

    DL::COLOR_MASK getWriteMask(GAPI::WriteMask mask)
    {
        DL::COLOR_MASK result = DL::COLOR_MASK_NONE;
        if (IsSet(mask, GAPI::WriteMask::Red))
            result |= DL::COLOR_MASK_RED;
        if (IsSet(mask, GAPI::WriteMask::Green))
            result |= DL::COLOR_MASK_GREEN;
        if (IsSet(mask, GAPI::WriteMask::Blue))
            result |= DL::COLOR_MASK_BLUE;
        if (IsSet(mask, GAPI::WriteMask::Alpha))
            result |= DL::COLOR_MASK_ALPHA;
        return result;
    }

    DL::RenderTargetBlendDesc getRenderTargetBlendDesc(const GAPI::RTBlendStateDesc& desc)
    {
        DL::RenderTargetBlendDesc blendDesc;
        blendDesc.BlendEnable = desc.blendEnabled;
        blendDesc.BlendOp = getBlendOp(desc.rgbBlendOp);
        blendDesc.BlendOpAlpha = getBlendOp(desc.alphaBlendOp);
        blendDesc.DestBlend = getBlendFunc(desc.dstRgb);
        blendDesc.SrcBlend = getBlendFunc(desc.srcRgb);
        blendDesc.DestBlendAlpha = getBlendFunc(desc.dstAlpha);
        blendDesc.SrcBlendAlpha = getBlendFunc(desc.srcAlpha);
        blendDesc.RenderTargetWriteMask = getWriteMask(desc.writeMask);
        return blendDesc;
    }

    DL::BlendStateDesc getBlendDesc(const GAPI::BlendDesc& desc)
    {
        DL::BlendStateDesc blendDesc;
        blendDesc.AlphaToCoverageEnable = desc.alphaToCoverageEnabled;
        blendDesc.IndependentBlendEnable = desc.independentBlendEnabled;
        for (size_t i = 0; i < desc.rtBlend.size(); i++)
        {
            blendDesc.RenderTargets[i] = getRenderTargetBlendDesc(desc.rtBlend[i]);
        }
        return blendDesc;
    }

    DL::VALUE_TYPE getValueType(GAPI::VertexAttributeType type)
    {
        switch (type)
        {
            case GAPI::VertexAttributeType::Float: return DL::VALUE_TYPE::VT_FLOAT32;
            case GAPI::VertexAttributeType::Half: return DL::VALUE_TYPE::VT_FLOAT16;

            case GAPI::VertexAttributeType::Uint8: return DL::VALUE_TYPE::VT_UINT8;
            case GAPI::VertexAttributeType::Int8: return DL::VALUE_TYPE::VT_INT8;
            case GAPI::VertexAttributeType::Unorm8: return DL::VALUE_TYPE::VT_UINT8;
            case GAPI::VertexAttributeType::Snorm8: return DL::VALUE_TYPE::VT_INT8;

            case GAPI::VertexAttributeType::Uint16: return DL::VALUE_TYPE::VT_UINT16;
            case GAPI::VertexAttributeType::Int16: return DL::VALUE_TYPE::VT_INT16;
            case GAPI::VertexAttributeType::Unorm16: return DL::VALUE_TYPE::VT_UINT16;
            case GAPI::VertexAttributeType::Snorm16: return DL::VALUE_TYPE::VT_INT16;

            case GAPI::VertexAttributeType::Uint32: return DL::VALUE_TYPE::VT_UINT32;
            case GAPI::VertexAttributeType::Int32: return DL::VALUE_TYPE::VT_INT32;
            case GAPI::VertexAttributeType::Unorm32: return DL::VALUE_TYPE::VT_UINT32;
            case GAPI::VertexAttributeType::Snorm32: return DL::VALUE_TYPE::VT_INT32;

            default:
                ASSERT_MSG(false, "Unknown vertex attribute type");
                return DL::VALUE_TYPE::VT_UNDEFINED;
        }
    }

    bool isNormalized(GAPI::VertexAttributeType type)
    {
        switch (type)
        {
            case GAPI::VertexAttributeType::Unorm8:
            case GAPI::VertexAttributeType::Unorm16:
            case GAPI::VertexAttributeType::Unorm32:
            case GAPI::VertexAttributeType::Snorm8:
            case GAPI::VertexAttributeType::Snorm16:
            case GAPI::VertexAttributeType::Snorm32:
            return true;

            default:
                return false;
        }
    }

    DL::InputLayoutDesc getInputLayout(const GAPI::VertexLayout& layout, eastl::array<DL::LayoutElement, MAX_VERTEX_ATTRIBUTES>& layoutElements)
    {
        DL::InputLayoutDesc inputLayout;

        ASSERT(layout.GetAttributeCount() <= MaxLayoutElements);

        for (size_t i = 0; i < layout.GetAttributeCount(); i++)
        {
            auto& layoutElement = layoutElements[i];
            const auto& attribute = layout.GetAttribute(i);

            layoutElement.HLSLSemantic = attribute.semanticName;
            layoutElement.InputIndex = attribute.semanticIndex;

            layoutElement.BufferSlot = attribute.bufferSlot;
            layoutElement.NumComponents = attribute.numComponents;
            layoutElement.ValueType = getValueType(attribute.type);
            layoutElement.IsNormalized = isNormalized(attribute.type);

            layoutElement.RelativeOffset = DL::LAYOUT_ELEMENT_AUTO_OFFSET;
            layoutElement.Stride = DL::LAYOUT_ELEMENT_AUTO_STRIDE;

            layoutElement.Frequency = DL::INPUT_ELEMENT_FREQUENCY_PER_VERTEX;
            layoutElement.InstanceDataStepRate = 1;
        }

        inputLayout.LayoutElements = layoutElements.data();
        inputLayout.NumElements = static_cast<uint32_t>(layout.GetAttributeCount());
        return inputLayout;
    }

    DL::GraphicsPipelineDesc getGraphicsPipelineDesc(const GAPI::GraphicPipelineStateDesc& desc, eastl::array<DL::LayoutElement, MAX_VERTEX_ATTRIBUTES>& layoutElements)
    {
        DL::GraphicsPipelineDesc dlDesc;
        dlDesc.BlendDesc = getBlendDesc(desc.blendDesc);
        dlDesc.NumViewports = 1;

        ASSERT(desc.colorAttachmentCount <= MAX_COLOR_ATTACHMENT_COUNT);
        ASSERT(desc.colorAttachmentCount <= DILIGENT_MAX_RENDER_TARGETS);

        uint32_t renderTargetCount = Min(desc.colorAttachmentCount, MAX_COLOR_ATTACHMENT_COUNT);
        renderTargetCount = Min<uint32_t>(renderTargetCount, DILIGENT_MAX_RENDER_TARGETS);

        dlDesc.NumRenderTargets = static_cast<uint8_t>(renderTargetCount);

        for (size_t i = 0; i < renderTargetCount; i++)
            dlDesc.RTVFormats[i] = GetDLTextureFormat(desc.colorAttachmentFormats[i]);

        dlDesc.DSVFormat = GetDLTextureFormat(desc.depthStencilFormat);
        dlDesc.InputLayout = getInputLayout(desc.vertexLayout, layoutElements);

        /*  dlDesc.RasterizerDesc = getRasterizerDesc(desc.rasterizerDesc);
          dlDesc.DepthStencilDesc = getDepthStencilDesc(desc.depthStencilDesc);
          dlDesc.InputLayout = getInputLayout(desc.inputLayout);
          dlDesc.PrimitiveTopology = getPrimitiveTopology(desc.primitiveTopology);
          dlDesc.NumViewports = desc.numViewports;
          dlDesc.NumRenderTargets = desc.numRenderTargets;*/
        return dlDesc;
    }

    DL::GraphicsPipelineStateCreateInfo getGraphicPipelineStateCreateInfo(
        const GAPI::GraphicPipelineStateDesc& desc,
        const std::string& name,
        eastl::array<DL::LayoutElement, MAX_VERTEX_ATTRIBUTES>& layoutElements)
    {
        DL::GraphicsPipelineStateCreateInfo createInfo(name.c_str());
        createInfo.GraphicsPipeline = getGraphicsPipelineDesc(desc, layoutElements);

        ASSERT_MSG(desc.vs, "VS is not set in pipeline state: \"{}\"", name);
        ASSERT_MSG(desc.ps, "PS is not set in pipeline state: \"{}\"", name);

        createInfo.pVS = static_cast<const ShaderImpl*>(desc.vs->GetPrivateImpl())->GetShader();
        createInfo.pPS = static_cast<const ShaderImpl*>(desc.ps->GetPrivateImpl())->GetShader();
        return createInfo;
    }

    PipelineStateImpl::~PipelineStateImpl() { };

    void PipelineStateImpl::Init(DL::IRenderDevice* device, GAPI::PipelineState& resource)
    {
        switch (resource.GetPsoType())
        {
            case PipelineState::PsoType::Graphic:
            {
                ASSERT(dynamic_cast<const GAPI::GraphicPipelineState*>(&resource));
                auto& graphicResource = static_cast<const GAPI::GraphicPipelineState&>(resource);

                // To avoid allocations and keep thread safity we should allocate this on stack
                eastl::array<DL::LayoutElement, MAX_VERTEX_ATTRIBUTES> layoutElements;
                auto createInfo = getGraphicPipelineStateCreateInfo(graphicResource.GetDescription(), resource.GetName(), layoutElements);

                DL::IPipelineState* psoPtr = nullptr;
                device->CreateGraphicsPipelineState(createInfo, &psoPtr);
                pso.Attach(psoPtr);

         //       device->CreatePipelineResourceSignature
                DL::IShaderResourceBinding *srbPtr = nullptr;
                psoPtr->CreateShaderResourceBinding(&srbPtr, false);

        //        DL::PipelineResourceDesc resourceDesc;

         //       device->CreateResourceMapping(resourceMapping, nullptr);

         //       srbPtr->BindResources(resourceMapping, DL::BIND_SHADER_RESOURCES_FLAGS_NONE);

                break;
            }
            default:
                ASSERT_MSG(false, "Unknown pipeline state type");
                return;
        }
    }

}