#include "PipelineStateImpl.hpp"

#include "RenderDevice.h"
#include "BlendState.h"

#include "gapi/PipelineState.hpp"

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

    DL::GraphicsPipelineDesc getGraphicsPipelineDesc(const GAPI::GraphicPipelineStateDesc& desc)
    {
        DL::GraphicsPipelineDesc dlDesc;
        dlDesc.BlendDesc = getBlendDesc(desc.blendDesc);
        dlDesc.NumViewports;

        ASSERT(desc.renderTargetCount <= MAX_RENDER_TARGETS_COUNT);
        ASSERT(desc.renderTargetCount <= DILIGENT_MAX_RENDER_TARGETS);

        uint32_t renderTargetCount = Min(desc.renderTargetCount, MAX_RENDER_TARGETS_COUNT);
        renderTargetCount = Min<uint32_t>(renderTargetCount, DILIGENT_MAX_RENDER_TARGETS);

        dlDesc.NumRenderTargets = renderTargetCount;

        for (size_t i = 0; i < renderTargetCount; i++)
            dlDesc.RTVFormats[i] = GetDLTextureFormat(desc.renderTargetFormats[i]);

        dlDesc.DSVFormat = GetDLTextureFormat(desc.depthStencilFormat);
        /*  dlDesc.RasterizerDesc = getRasterizerDesc(desc.rasterizerDesc);
          dlDesc.DepthStencilDesc = getDepthStencilDesc(desc.depthStencilDesc);
          dlDesc.InputLayout = getInputLayout(desc.inputLayout);
          dlDesc.PrimitiveTopology = getPrimitiveTopology(desc.primitiveTopology);
          dlDesc.NumViewports = desc.numViewports;
          dlDesc.NumRenderTargets = desc.numRenderTargets;*/
        return dlDesc;
    }


    DL::GraphicsPipelineStateCreateInfo getGraphicPipelineStateCreateInfo(const GAPI::GraphicPipelineStateDesc& desc, const std::string& name)
    {
        DL::GraphicsPipelineStateCreateInfo createInfo(name.c_str());
        createInfo.GraphicsPipeline = getGraphicsPipelineDesc(desc);
        createInfo.pVS = static_cast<ShaderImpl*>(desc.vs->GetPrivateImpl())->GetShader();
        createInfo.pPS = static_cast<ShaderImpl*>(desc.ps->GetPrivateImpl())->GetShader();
        return createInfo;
    }

    PipelineStateImpl::~PipelineStateImpl() { };

    void PipelineStateImpl::Init(DL::IRenderDevice* device, GAPI::PipelineState& resource) const
    {
        switch (resource.GetPsoType())
        {
            case PipelineState::PsoType::Graphic:
            {
                ASSERT(dynamic_cast<const GAPI::GraphicPipelineState*>(&resource));
                auto& graphicResource = static_cast<const GAPI::GraphicPipelineState&>(resource);
                auto createInfo = getGraphicPipelineStateCreateInfo(graphicResource.GetDescription(), resource.GetName());

                DL::IPipelineState* pso = nullptr;
                device->CreateGraphicsPipelineState(createInfo, &pso);
                break;
            }
            default:
                ASSERT_MSG(false, "Unknown pipeline state type");
                return;
        }
    }

}