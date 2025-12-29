#include "PipelineStateImpl.hpp"

#include "gapi/PipelineState.hpp"
#include "gapi/VertexLayout.hpp"

#include "common/Result.hpp"

#include "ShaderImpl.hpp"
#include "Utils.hpp"

namespace RR::GAPI::WebGPU
{
    static constexpr uint32_t MaxLayoutElements = 16;

    wgpu::BlendOperation getBlendOp(GAPI::BlendOp op)
    {
        switch (op)
        {
            case GAPI::BlendOp::Add: return wgpu::BlendOperation::Add;
            case GAPI::BlendOp::Subtract: return wgpu::BlendOperation::Subtract;
            case GAPI::BlendOp::ReverseSubtract: return wgpu::BlendOperation::ReverseSubtract;
            case GAPI::BlendOp::Min: return wgpu::BlendOperation::Min;
            case GAPI::BlendOp::Max: return wgpu::BlendOperation::Max;
            default:
                ASSERT_MSG(false, "Unknown blend op");
                return wgpu::BlendOperation::Add;
        }
    }

    wgpu::BlendFactor getBlendFactor(GAPI::BlendFactor factor)
    {
        switch (factor)
        {
            case GAPI::BlendFactor::Zero: return wgpu::BlendFactor::Zero;
            case GAPI::BlendFactor::One: return wgpu::BlendFactor::One;
            case GAPI::BlendFactor::SrcColor: return wgpu::BlendFactor::Src;
            case GAPI::BlendFactor::OneMinusSrcColor: return wgpu::BlendFactor::OneMinusSrc;
            case GAPI::BlendFactor::DstColor: return wgpu::BlendFactor::Dst;
            case GAPI::BlendFactor::OneMinusDstColor: return wgpu::BlendFactor::OneMinusDst;
            case GAPI::BlendFactor::SrcAlpha: return wgpu::BlendFactor::SrcAlpha;
            case GAPI::BlendFactor::OneMinusSrcAlpha: return wgpu::BlendFactor::OneMinusSrcAlpha;
            case GAPI::BlendFactor::DstAlpha: return wgpu::BlendFactor::DstAlpha;
            case GAPI::BlendFactor::OneMinusDstAlpha: return wgpu::BlendFactor::OneMinusDstAlpha;
            case GAPI::BlendFactor::SrcAlphaSaturate: return wgpu::BlendFactor::SrcAlphaSaturated;
            case GAPI::BlendFactor::Src1Color: return wgpu::BlendFactor::Src1;
            case GAPI::BlendFactor::OneMinusSrc1Color: return wgpu::BlendFactor::OneMinusSrc1;
            case GAPI::BlendFactor::Src1Alpha: return wgpu::BlendFactor::Src1Alpha;
            case GAPI::BlendFactor::OneMinusSrc1Alpha: return wgpu::BlendFactor::OneMinusSrc1Alpha;
            default:
                ASSERT_MSG(false, "Unknown blend factor");
                return wgpu::BlendFactor::One;
        }
    }

    wgpu::ColorWriteMask getColorWriteMask(GAPI::WriteMask mask)
    {
        wgpu::ColorWriteMask result = wgpu::ColorWriteMask::None;
        if (IsSet(mask, GAPI::WriteMask::Red))
            result = result | wgpu::ColorWriteMask::Red;
        if (IsSet(mask, GAPI::WriteMask::Green))
            result = result | wgpu::ColorWriteMask::Green;
        if (IsSet(mask, GAPI::WriteMask::Blue))
            result = result | wgpu::ColorWriteMask::Blue;
        if (IsSet(mask, GAPI::WriteMask::Alpha))
            result = result | wgpu::ColorWriteMask::Alpha;
        return result;
    }

    RR::Common::RResult getVertexFormat(GAPI::VertexAttributeType type, uint16_t numComponents, wgpu::VertexFormat& outFormat)
    {
        switch (type)
        {
            case GAPI::VertexAttributeType::Float:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Float32; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Float32x2; return RR::Common::RResult::Ok;
                    case 3: outFormat = wgpu::VertexFormat::Float32x3; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Float32x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Half:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Float16; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Float16x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Float16x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Uint8:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Uint8; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Uint8x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Uint8x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Unorm8:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Unorm8; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Unorm8x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Unorm8x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Int8:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Sint8; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Sint8x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Sint8x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Snorm8:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Snorm8; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Snorm8x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Snorm8x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Uint16:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Uint16; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Uint16x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Uint16x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Unorm16:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Unorm16; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Unorm16x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Unorm16x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Int16:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Sint16; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Sint16x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Sint16x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Snorm16:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Snorm16; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Snorm16x2; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Snorm16x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Uint32:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Uint32; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Uint32x2; return RR::Common::RResult::Ok;
                    case 3: outFormat = wgpu::VertexFormat::Uint32x3; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Uint32x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Int32:
                switch (numComponents)
                {
                    case 1: outFormat = wgpu::VertexFormat::Sint32; return RR::Common::RResult::Ok;
                    case 2: outFormat = wgpu::VertexFormat::Sint32x2; return RR::Common::RResult::Ok;
                    case 3: outFormat = wgpu::VertexFormat::Sint32x3; return RR::Common::RResult::Ok;
                    case 4: outFormat = wgpu::VertexFormat::Sint32x4; return RR::Common::RResult::Ok;
                    default: return RR::Common::RResult::InvalidArgument;
                }
            case GAPI::VertexAttributeType::Unorm32:
            case GAPI::VertexAttributeType::Snorm32:
                return RR::Common::RResult::NotAvailable; // WebGPU doesn't support Unorm32/Snorm32
            default:
                return RR::Common::RResult::InvalidArgument;
        }
    }

    wgpu::PrimitiveTopology getPrimitiveTopology(GAPI::PrimitiveTopology topology)
    {
        switch (topology)
        {
            case GAPI::PrimitiveTopology::PointList: return wgpu::PrimitiveTopology::PointList;
            case GAPI::PrimitiveTopology::LineList: return wgpu::PrimitiveTopology::LineList;
            case GAPI::PrimitiveTopology::LineStrip: return wgpu::PrimitiveTopology::LineStrip;
            case GAPI::PrimitiveTopology::TriangleList: return wgpu::PrimitiveTopology::TriangleList;
            case GAPI::PrimitiveTopology::TriangleStrip: return wgpu::PrimitiveTopology::TriangleStrip;
            default:
                Log::Format::Error("Unsupported primitive topology: {}", static_cast<uint32_t>(topology));
                return wgpu::PrimitiveTopology::TriangleList;
        }
    }

    wgpu::CullMode getCullMode(GAPI::CullMode cullMode)
    {
        switch (cullMode)
        {
            case GAPI::CullMode::None: return wgpu::CullMode::None;
            case GAPI::CullMode::Front: return wgpu::CullMode::Front;
            case GAPI::CullMode::Back: return wgpu::CullMode::Back;
            default:
                ASSERT_MSG(false, "Unknown cull mode");
                return wgpu::CullMode::None;
        }
    }

    wgpu::FrontFace getFrontFace(bool isFrontCcw)
    {
        return isFrontCcw ? wgpu::FrontFace::CCW : wgpu::FrontFace::CW;
    }

    wgpu::CompareFunction getCompareFunction(GAPI::DepthStencilDesc::ComparisonFunc func)
    {
        switch (func)
        {
            case GAPI::DepthStencilDesc::ComparisonFunc::Never: return wgpu::CompareFunction::Never;
            case GAPI::DepthStencilDesc::ComparisonFunc::Always: return wgpu::CompareFunction::Always;
            case GAPI::DepthStencilDesc::ComparisonFunc::Equal: return wgpu::CompareFunction::Equal;
            case GAPI::DepthStencilDesc::ComparisonFunc::NotEqual: return wgpu::CompareFunction::NotEqual;
            case GAPI::DepthStencilDesc::ComparisonFunc::Less: return wgpu::CompareFunction::Less;
            case GAPI::DepthStencilDesc::ComparisonFunc::LessEqual: return wgpu::CompareFunction::LessEqual;
            case GAPI::DepthStencilDesc::ComparisonFunc::Greater: return wgpu::CompareFunction::Greater;
            case GAPI::DepthStencilDesc::ComparisonFunc::GreaterEqual: return wgpu::CompareFunction::GreaterEqual;
            default:
                ASSERT_MSG(false, "Unknown comparison function");
                return wgpu::CompareFunction::Always;
        }
    }

    wgpu::StencilOperation getStencilOperation(GAPI::DepthStencilDesc::StencilOp op)
    {
        switch (op)
        {
            case GAPI::DepthStencilDesc::StencilOp::Keep: return wgpu::StencilOperation::Keep;
            case GAPI::DepthStencilDesc::StencilOp::Zero: return wgpu::StencilOperation::Zero;
            case GAPI::DepthStencilDesc::StencilOp::Replace: return wgpu::StencilOperation::Replace;
            case GAPI::DepthStencilDesc::StencilOp::Increase: return wgpu::StencilOperation::IncrementWrap;
            case GAPI::DepthStencilDesc::StencilOp::IncreaseSaturate: return wgpu::StencilOperation::IncrementClamp;
            case GAPI::DepthStencilDesc::StencilOp::Decrease: return wgpu::StencilOperation::DecrementWrap;
            case GAPI::DepthStencilDesc::StencilOp::DecreaseSaturate: return wgpu::StencilOperation::DecrementClamp;
            case GAPI::DepthStencilDesc::StencilOp::Invert: return wgpu::StencilOperation::Invert;
            default:
                ASSERT_MSG(false, "Unknown stencil operation");
                return wgpu::StencilOperation::Keep;
        }
    }

    void buildVertexBufferLayouts(
        const GAPI::VertexLayout& layout,
        eastl::fixed_vector<wgpu::VertexBufferLayout, MAX_VERTEX_BUFFERS, false>& bufferLayouts,
        eastl::array<eastl::array<wgpu::VertexAttribute, 8>, MaxLayoutElements>& attributesPerBuffer)
    {
        // Group attributes by buffer slot
        eastl::array<uint32_t, MaxLayoutElements> attributeCounts = {};
        eastl::array<uint32_t, MaxLayoutElements> attributeOffsets = {};

        for (size_t i = 0; i < layout.GetAttributeCount(); i++)
        {
            const auto& attribute = layout.GetAttribute(i);
            ASSERT(attribute.bufferSlot < MaxLayoutElements);

            uint32_t slot = attribute.bufferSlot;
            uint32_t index = attributeCounts[slot];

            attributesPerBuffer[slot][index].setDefault();

            wgpu::VertexFormat vertexFormat;
            if (RR_FAILED(getVertexFormat(attribute.type, attribute.numComponents, vertexFormat)))
                ASSERT_MSG(false, "Failed to get vertex format");

            attributesPerBuffer[slot][index].format = vertexFormat;
            attributesPerBuffer[slot][index].offset = attributeOffsets[slot];
            attributesPerBuffer[slot][index].shaderLocation = static_cast<uint32_t>(i);

            // Calculate offset increment (simplified - assumes packed layout)
            uint32_t componentSize;
            switch (attribute.type)
            {
                case GAPI::VertexAttributeType::Uint8:
                case GAPI::VertexAttributeType::Int8:
                case GAPI::VertexAttributeType::Unorm8:
                case GAPI::VertexAttributeType::Snorm8:
                    componentSize = 1;
                    break;
                case GAPI::VertexAttributeType::Uint16:
                case GAPI::VertexAttributeType::Int16:
                case GAPI::VertexAttributeType::Unorm16:
                case GAPI::VertexAttributeType::Snorm16:
                case GAPI::VertexAttributeType::Half:
                    componentSize = 2;
                    break;
                case GAPI::VertexAttributeType::Float:
                case GAPI::VertexAttributeType::Uint32:
                case GAPI::VertexAttributeType::Int32:
                    componentSize = 4;
                    break;
                default:
                    ASSERT_MSG(false, "Unknown vertex attribute type for component size calculation");
                    componentSize = 4;
                    break;
            }

            attributeOffsets[slot] += componentSize * attribute.numComponents;
            attributeCounts[slot]++;
        }

        // Build buffer layouts
        for (uint32_t slot = 0; slot < MaxLayoutElements; slot++)
        {
            if (attributeCounts[slot] > 0)
            {
                auto& bufferLayout = bufferLayouts.emplace_back();
                bufferLayout.setDefault();
                bufferLayout.arrayStride = attributeOffsets[slot];
                bufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
                bufferLayout.attributeCount = attributeCounts[slot];
                bufferLayout.attributes = attributesPerBuffer[slot].data();
            }
        }
    }

    PipelineStateImpl::~PipelineStateImpl() { }

    void PipelineStateImpl::Init(const wgpu::Device& device, GAPI::PipelineState& resource)
    {
        switch (resource.GetPsoType())
        {
            case GAPI::PipelineState::PsoType::Graphic:
            {
                ASSERT(dynamic_cast<const GAPI::GraphicPipelineState*>(&resource));
                auto& graphicResource = static_cast<const GAPI::GraphicPipelineState&>(resource);
                const auto& desc = graphicResource.GetDescription();

                ASSERT_MSG(desc.vs, "VS is not set in pipeline state: \"{}\"", resource.GetName());
                ASSERT_MSG(desc.ps, "PS is not set in pipeline state: \"{}\"", resource.GetName());

                const auto* vsImpl = static_cast<const ShaderImpl*>(desc.vs->GetPrivateImpl());
                const auto* psImpl = static_cast<const ShaderImpl*>(desc.ps->GetPrivateImpl());

                // Build vertex buffer layouts
                eastl::fixed_vector<wgpu::VertexBufferLayout, MAX_VERTEX_BUFFERS, false> bufferLayouts;
                eastl::array<eastl::array<wgpu::VertexAttribute, 8>, MaxLayoutElements> attributesPerBuffer;

                if (desc.vertexLayout.GetAttributeCount() > 0)
                    buildVertexBufferLayouts(desc.vertexLayout, bufferLayouts, attributesPerBuffer);

                // Build depth stencil state
                wgpu::DepthStencilState depthStencilState;
                if (desc.depthStencilFormat != GAPI::GpuResourceFormat::Unknown)
                {
                    depthStencilState.setDefault();
                    depthStencilState.format = GetWGPUFormat(desc.depthStencilFormat);
                    depthStencilState.depthWriteEnabled = IsSet(desc.depthStencilDesc.depthAccess, GAPI::DepthStencilDesc::DepthAccess::Write)
                                                              ? wgpu::OptionalBool::True
                                                              : wgpu::OptionalBool::False;

                    if (IsSet(desc.depthStencilDesc.depthAccess, GAPI::DepthStencilDesc::DepthAccess::Read))
                    {
                        depthStencilState.depthCompare = getCompareFunction(desc.depthStencilDesc.depthFunc);
                    }
                    else
                        depthStencilState.depthCompare = wgpu::CompareFunction::Always;

                    if (desc.depthStencilDesc.stencilEnabled)
                    {
                        depthStencilState.stencilFront.compare = getCompareFunction(desc.depthStencilDesc.stencilFront.func);
                        depthStencilState.stencilFront.failOp = getStencilOperation(desc.depthStencilDesc.stencilFront.stencilFailOp);
                        depthStencilState.stencilFront.depthFailOp = getStencilOperation(desc.depthStencilDesc.stencilFront.depthFailOp);
                        depthStencilState.stencilFront.passOp = getStencilOperation(desc.depthStencilDesc.stencilFront.depthStencilPassOp);

                        depthStencilState.stencilBack.compare = getCompareFunction(desc.depthStencilDesc.stencilBack.func);
                        depthStencilState.stencilBack.failOp = getStencilOperation(desc.depthStencilDesc.stencilBack.stencilFailOp);
                        depthStencilState.stencilBack.depthFailOp = getStencilOperation(desc.depthStencilDesc.stencilBack.depthFailOp);
                        depthStencilState.stencilBack.passOp = getStencilOperation(desc.depthStencilDesc.stencilBack.depthStencilPassOp);
                    }

                    depthStencilState.stencilReadMask = desc.depthStencilDesc.stencilReadMask;
                    depthStencilState.stencilWriteMask = desc.depthStencilDesc.stencilWriteMask;
                }

                // Build color targets
                eastl::array<wgpu::ColorTargetState, MAX_COLOR_ATTACHMENT_COUNT> colorTargets;
                eastl::array<wgpu::BlendState, MAX_COLOR_ATTACHMENT_COUNT> blendStates;

                for (uint32_t i = 0; i < desc.colorAttachmentCount; i++)
                {
                    auto& colorTarget = colorTargets[i];
                    auto& blendState = blendStates[i];
                    const auto& rtBlend = desc.blendDesc.rtBlend[i];
                    colorTarget.setDefault();
                    colorTarget.format = GetWGPUFormat(desc.colorAttachmentFormats[i]);

                    if (rtBlend.blendEnabled)
                    {
                        blendState.setDefault();
                        blendState.color.operation = getBlendOp(rtBlend.rgbBlendOp);
                        blendState.color.srcFactor = getBlendFactor(rtBlend.srcRgb);
                        blendState.color.dstFactor = getBlendFactor(rtBlend.dstRgb);
                        blendState.alpha.operation = getBlendOp(rtBlend.alphaBlendOp);
                        blendState.alpha.srcFactor = getBlendFactor(rtBlend.srcAlpha);
                        blendState.alpha.dstFactor = getBlendFactor(rtBlend.dstAlpha);
                    }

                    colorTarget.blend = rtBlend.blendEnabled ? &blendState : nullptr;
                    colorTarget.writeMask = getColorWriteMask(rtBlend.writeMask);
                }


                // Build fragment state
                wgpu::FragmentState fragmentState;
                fragmentState.module = psImpl->GetShaderModule();
                fragmentState.entryPoint = wgpu::StringView("main");
                fragmentState.targetCount = desc.colorAttachmentCount;
                fragmentState.targets = colorTargets.data();

                // Build render pipeline descriptor
                wgpu::RenderPipelineDescriptor pipelineDesc;
                pipelineDesc.setDefault();
                pipelineDesc.label = wgpu::StringView(resource.GetName().c_str());

                // Vertex state
                pipelineDesc.vertex.module = vsImpl->GetShaderModule();
                pipelineDesc.vertex.entryPoint = wgpu::StringView("main");
                pipelineDesc.vertex.bufferCount = bufferLayouts.size();
                pipelineDesc.vertex.buffers = bufferLayouts.size() > 0 ? bufferLayouts.data() : nullptr;

                // Primitive state
                pipelineDesc.primitive.topology = getPrimitiveTopology(desc.primitiveTopology);
                pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
                pipelineDesc.primitive.frontFace = getFrontFace(desc.rasterizerDesc.isFrontCcw);
                pipelineDesc.primitive.cullMode = getCullMode(desc.rasterizerDesc.cullMode);

                // Fragment state
                pipelineDesc.fragment = &fragmentState;

                // Depth stencil state
                if (desc.depthStencilFormat != GAPI::GpuResourceFormat::Unknown)
                {
                    pipelineDesc.depthStencil = &depthStencilState;
                }

                // Multisample state
                // TODO multisample support
                pipelineDesc.multisample.count = 1;
                pipelineDesc.multisample.mask = UINT_MAX;
                pipelineDesc.multisample.alphaToCoverageEnabled = desc.blendDesc.alphaToCoverageEnabled;

                renderPipeline = device.createRenderPipeline(pipelineDesc);
                break;
            }
            default:
                ASSERT_MSG(false, "Unknown pipeline state type");
                return;
        }
    }
}
