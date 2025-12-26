#include "PipelineStateImpl.hpp"

#include "gapi/PipelineState.hpp"

#include "gapi_dx12/DeviceContext.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR
{
    namespace GAPI::DX12
    {
        struct InputLayout
        {
            enum class InputClass : uint32_t
            {
                PerVertex,
                PerInstance
            };

            struct InputStreamLayout
            {
                struct Element
                {
                    std::string semanticName;
                    GpuResourceFormat format;
                    uint32_t arraySize;
                    uint32_t alignedByteOffset;
                };

                std::vector<Element> elements;
                InputClass inputClass;
                uint32_t instanceStepRate;
                uint32_t vertexStride;
            };

            std::vector<InputStreamLayout> inputStreams;
        };

        namespace
        {
            D3D12_BLEND getD3D12BlendFunc(BlendFactor func)
            {
                switch (func)
                {
                    case BlendFactor::Zero: return D3D12_BLEND_ZERO;
                    case BlendFactor::One: return D3D12_BLEND_ONE;
                    case BlendFactor::SrcColor: return D3D12_BLEND_SRC_COLOR;
                    case BlendFactor::OneMinusSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
                    case BlendFactor::DstColor: return D3D12_BLEND_DEST_COLOR;
                    case BlendFactor::OneMinusDstColor: return D3D12_BLEND_INV_DEST_COLOR;
                    case BlendFactor::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
                    case BlendFactor::OneMinusSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
                    case BlendFactor::DstAlpha: return D3D12_BLEND_DEST_ALPHA;
                    case BlendFactor::OneMinusDstAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
                    case BlendFactor::SrcAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
                    case BlendFactor::Src1Color: return D3D12_BLEND_INV_SRC1_COLOR;
                    case BlendFactor::OneMinusSrc1Color: return D3D12_BLEND_INV_SRC1_COLOR;
                    case BlendFactor::Src1Alpha: return D3D12_BLEND_SRC1_ALPHA;
                    case BlendFactor::OneMinusSrc1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
                    default: ASSERT_MSG(false, "Unknown blend func"); return (D3D12_BLEND)0;
                }
            }

            D3D12_BLEND_OP getD3D12BlendOp(BlendOp blendOp)
            {
                switch (blendOp)
                {
                    case BlendOp::Add: return D3D12_BLEND_OP_ADD;
                    case BlendOp::Subtract: return D3D12_BLEND_OP_SUBTRACT;
                    case BlendOp::ReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
                    case BlendOp::Min: return D3D12_BLEND_OP_MIN;
                    case BlendOp::Max: return D3D12_BLEND_OP_MAX;
                    default: ASSERT_MSG(false, "Unknown blend op"); return (D3D12_BLEND_OP)0;
                }
            }

            D3D12_FILL_MODE getD3DFillMode(FillMode fillmode)
            {
                switch (fillmode)
                {
                    case FillMode::Solid: return D3D12_FILL_MODE_SOLID;
                    case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
                    default: ASSERT_MSG(false, "Unknown fill mode"); return D3D12_FILL_MODE(0);
                }
            }

            D3D12_CULL_MODE getD3DCullMode(CullMode fillmode)
            {
                switch (fillmode)
                {
                    case CullMode::None: return D3D12_CULL_MODE_NONE;
                    case CullMode::Front: return D3D12_CULL_MODE_FRONT;
                    case CullMode::Back: return D3D12_CULL_MODE_BACK;
                    default: ASSERT_MSG(false, "Unknown cull mode"); return D3D12_CULL_MODE_NONE;
                }
            }

            D3D12_COMPARISON_FUNC getD3D12ComparisonFunc(DepthStencilDesc::ComparisonFunc func)
            {
                switch (func)
                {
                    case DepthStencilDesc::ComparisonFunc::Never: return D3D12_COMPARISON_FUNC_NEVER;
                    case DepthStencilDesc::ComparisonFunc::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
                    case DepthStencilDesc::ComparisonFunc::Less: return D3D12_COMPARISON_FUNC_LESS;
                    case DepthStencilDesc::ComparisonFunc::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
                    case DepthStencilDesc::ComparisonFunc::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                    case DepthStencilDesc::ComparisonFunc::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                    case DepthStencilDesc::ComparisonFunc::Greater: return D3D12_COMPARISON_FUNC_GREATER;
                    case DepthStencilDesc::ComparisonFunc::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                    default: ASSERT_MSG(false, "Unknown comparison func"); return (D3D12_COMPARISON_FUNC)0;
                }
            }

            D3D12_STENCIL_OP getD3D12StencilOp(DepthStencilDesc::StencilOp op)
            {
                switch (op)
                {
                    case DepthStencilDesc::StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
                    case DepthStencilDesc::StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
                    case DepthStencilDesc::StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
                    case DepthStencilDesc::StencilOp::Increase: return D3D12_STENCIL_OP_INCR;
                    case DepthStencilDesc::StencilOp::IncreaseSaturate: return D3D12_STENCIL_OP_INCR_SAT;
                    case DepthStencilDesc::StencilOp::Decrease: return D3D12_STENCIL_OP_DECR;
                    case DepthStencilDesc::StencilOp::DecreaseSaturate: return D3D12_STENCIL_OP_DECR_SAT;
                    case DepthStencilDesc::StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
                    default: ASSERT_MSG(false, "Unknown stencil op"); return (D3D12_STENCIL_OP)0;
                }
            }

            D3D12_PRIMITIVE_TOPOLOGY_TYPE getD3DPrimitiveTopologyType(PrimitiveTopology topology)
            {
                switch (topology)
                {
                    case PrimitiveTopology::PointList:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

                    case PrimitiveTopology::LineList:
                    case PrimitiveTopology::LineStrip:
                    case PrimitiveTopology::LineListAdj:
                    case PrimitiveTopology::LineStripAdj:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

                    case PrimitiveTopology::TriangleList:
                    case PrimitiveTopology::TriangleStrip:
                    case PrimitiveTopology::TriangleListAdj:
                    case PrimitiveTopology::TriangleStripAdj:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

                    case PrimitiveTopology::PathListControlPoint_1:
                    case PrimitiveTopology::PathListControlPoint_2:
                    case PrimitiveTopology::PathListControlPoint_3:
                    case PrimitiveTopology::PathListControlPoint_4:
                    case PrimitiveTopology::PathListControlPoint_5:
                    case PrimitiveTopology::PathListControlPoint_6:
                    case PrimitiveTopology::PathListControlPoint_7:
                    case PrimitiveTopology::PathListControlPoint_8:
                    case PrimitiveTopology::PathListControlPoint_9:
                    case PrimitiveTopology::PathListControlPoint_10:
                    case PrimitiveTopology::PathListControlPoint_11:
                    case PrimitiveTopology::PathListControlPoint_12:
                    case PrimitiveTopology::PathListControlPoint_13:
                    case PrimitiveTopology::PathListControlPoint_14:
                    case PrimitiveTopology::PathListControlPoint_15:
                    case PrimitiveTopology::PathListControlPoint_16:
                    case PrimitiveTopology::PathListControlPoint_17:
                    case PrimitiveTopology::PathListControlPoint_18:
                    case PrimitiveTopology::PathListControlPoint_19:
                    case PrimitiveTopology::PathListControlPoint_20:
                    case PrimitiveTopology::PathListControlPoint_21:
                    case PrimitiveTopology::PathListControlPoint_22:
                    case PrimitiveTopology::PathListControlPoint_23:
                    case PrimitiveTopology::PathListControlPoint_24:
                    case PrimitiveTopology::PathListControlPoint_25:
                    case PrimitiveTopology::PathListControlPoint_26:
                    case PrimitiveTopology::PathListControlPoint_27:
                    case PrimitiveTopology::PathListControlPoint_28:
                    case PrimitiveTopology::PathListControlPoint_29:
                    case PrimitiveTopology::PathListControlPoint_30:
                    case PrimitiveTopology::PathListControlPoint_31:
                    case PrimitiveTopology::PathListControlPoint_32:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

                    default: ASSERT_MSG(false, "Unknown topology"); return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                }
            }

            D3D12_INPUT_CLASSIFICATION getD3DInputClass(InputLayout::InputClass inputClass)
            {
                switch (inputClass)
                {
                    case InputLayout::InputClass::PerVertex: return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    case InputLayout::InputClass::PerInstance: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                    default: ASSERT_MSG(false, "Unknown input class"); return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                }
            }

            void initResterizerDesc(const RasterizerDesc& rasterizerDesc, D3D12_RASTERIZER_DESC& d3dRasterizerDesc)
            {
                d3dRasterizerDesc.FillMode = getD3DFillMode(rasterizerDesc.fillMode);
                d3dRasterizerDesc.CullMode = getD3DCullMode(rasterizerDesc.cullMode);
                d3dRasterizerDesc.FrontCounterClockwise = rasterizerDesc.isFrontCcw;
                d3dRasterizerDesc.DepthBias = rasterizerDesc.depthBias;
                d3dRasterizerDesc.DepthBiasClamp = 0;
                d3dRasterizerDesc.SlopeScaledDepthBias = rasterizerDesc.slopeScaledDepthBias;
                d3dRasterizerDesc.DepthClipEnable = rasterizerDesc.depthClipEnabled;
                d3dRasterizerDesc.MultisampleEnable = rasterizerDesc.multisampleEnabled;
                d3dRasterizerDesc.AntialiasedLineEnable = rasterizerDesc.linesAAEnabled;
                d3dRasterizerDesc.ForcedSampleCount = rasterizerDesc.forcedSampleCount;
                d3dRasterizerDesc.ConservativeRaster = rasterizerDesc.conservativeRasterEnabled ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            }

            void initBlendDesc(const BlendDesc& blendDesc, D3D12_BLEND_DESC& d3dBlendDesc)
            {
                d3dBlendDesc.AlphaToCoverageEnable = blendDesc.alphaToCoverageEnabled;
                d3dBlendDesc.IndependentBlendEnable = blendDesc.independentBlendEnabled;

                static_assert(MAX_COLOR_ATTACHMENT_COUNT == std::size(D3D12_BLEND_DESC {}.RenderTarget));

                // If IndependentBlendEnable set to FALSE, only the RenderTarget[0] members are used
                uint32_t numRTs = d3dBlendDesc.IndependentBlendEnable ? blendDesc.rtBlend.size() : 1;

                for (uint32_t rt = 0; rt < numRTs; ++rt)
                {
                    const auto& rtDesc = blendDesc.rtBlend[rt];
                    auto& d3dRtDesc = d3dBlendDesc.RenderTarget[rt];

                    d3dRtDesc.LogicOpEnable = FALSE;
                    d3dRtDesc.LogicOp = D3D12_LOGIC_OP_SET;

                    d3dRtDesc.BlendEnable = rtDesc.blendEnabled;
                    d3dRtDesc.BlendOp = getD3D12BlendOp(rtDesc.rgbBlendOp);
                    d3dRtDesc.SrcBlend = getD3D12BlendFunc(rtDesc.srcRgb);
                    d3dRtDesc.DestBlend = getD3D12BlendFunc(rtDesc.dstRgb);
                    d3dRtDesc.BlendOpAlpha = getD3D12BlendOp(rtDesc.alphaBlendOp);
                    d3dRtDesc.SrcBlendAlpha = getD3D12BlendFunc(rtDesc.srcAlpha);
                    d3dRtDesc.DestBlendAlpha = getD3D12BlendFunc(rtDesc.dstAlpha);

                    d3dRtDesc.RenderTargetWriteMask = IsSet(rtDesc.writeMask, WriteMask::Red) ? D3D12_COLOR_WRITE_ENABLE_RED : 0;
                    d3dRtDesc.RenderTargetWriteMask |= IsSet(rtDesc.writeMask, WriteMask::Green) ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0;
                    d3dRtDesc.RenderTargetWriteMask |= IsSet(rtDesc.writeMask, WriteMask::Blue) ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0;
                    d3dRtDesc.RenderTargetWriteMask |= IsSet(rtDesc.writeMask, WriteMask::Alpha) ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0;
                }
            }

            D3D12_DEPTH_STENCILOP_DESC getD3D12StencilOpDesc(const DepthStencilDesc::StencilDesc& stencilDesc)
            {
                D3D12_DEPTH_STENCILOP_DESC result;
                result.StencilFailOp = getD3D12StencilOp(stencilDesc.stencilFailOp);
                result.StencilDepthFailOp = getD3D12StencilOp(stencilDesc.depthFailOp);
                result.StencilPassOp = getD3D12StencilOp(stencilDesc.depthStencilPassOp);
                result.StencilFunc = getD3D12ComparisonFunc(stencilDesc.func);
                return result;
            }

            void initDepthStencilDesc(const DepthStencilDesc& depthStencilDesc, D3D12_DEPTH_STENCIL_DESC& d3dDepthStencilDesc)
            {
                d3dDepthStencilDesc.DepthEnable = IsSet(depthStencilDesc.depthAccess, DepthStencilDesc::DepthAccess::Read);
                d3dDepthStencilDesc.DepthFunc = getD3D12ComparisonFunc(depthStencilDesc.depthFunc);
                d3dDepthStencilDesc.DepthWriteMask =
                    IsSet(depthStencilDesc.depthAccess, DepthStencilDesc::DepthAccess::Write) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
                d3dDepthStencilDesc.StencilEnable = depthStencilDesc.stencilEnabled;
                d3dDepthStencilDesc.StencilReadMask = depthStencilDesc.stencilReadMask;
                d3dDepthStencilDesc.StencilWriteMask = depthStencilDesc.stencilWriteMask;
                d3dDepthStencilDesc.FrontFace = getD3D12StencilOpDesc(depthStencilDesc.stencilFront);
                d3dDepthStencilDesc.BackFace = getD3D12StencilOpDesc(depthStencilDesc.stencilBack);
            }

            void initInputLayoutDesc(const InputLayout& inputLayout,
                                     D3D12_INPUT_LAYOUT_DESC& d3dInputLayout,
                                     std::vector<D3D12_INPUT_ELEMENT_DESC>& d3dElementDescs,
                                     std::vector<std::string>& semanticNames)
            {
                for (uint32_t streamIndex = 0; inputLayout.inputStreams.size(); ++streamIndex)
                {
                    const auto& streamLayout = inputLayout.inputStreams[streamIndex];
                    const auto d3dInputClass = getD3DInputClass(streamLayout.inputClass);

                    for (const auto& element : streamLayout.elements)
                    {
                        D3D12_INPUT_ELEMENT_DESC d3dElementDesc;
                        semanticNames.push_back(element.semanticName);
                        d3dElementDesc.SemanticName = semanticNames.back().c_str();
                        d3dElementDesc.Format = D3DUtils::GetDxgiResourceFormat(element.format);
                        d3dElementDesc.InputSlot = streamIndex;
                        d3dElementDesc.AlignedByteOffset = element.alignedByteOffset;
                        d3dElementDesc.InputSlotClass = d3dInputClass;
                        d3dElementDesc.InstanceDataStepRate = streamLayout.instanceStepRate;

                        for (uint32_t semanticIndex = 0; semanticIndex < element.arraySize; ++semanticIndex)
                        {
                            d3dElementDesc.SemanticIndex = semanticIndex;
                            d3dElementDesc.AlignedByteOffset += GpuResourceFormatInfo::GetBlockSize(element.format);
                            d3dElementDescs.push_back(d3dElementDesc);
                        }
                    }
                }

                d3dInputLayout.NumElements = d3dElementDescs.size();
                d3dInputLayout.pInputElementDescs = d3dInputLayout.NumElements > 0 ? d3dElementDescs.data() : nullptr;
            }
        }

        void PipelineStateImpl::Init()
        {
            GraphicPipelineStateDesc gpsoDesc;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12psoDesc = {};
            /* psoDesc.pRootSignature = rootSignature;
                psoDesc.VS = getShaderByteCode(vs);
                psoDesc.DS = getShaderByteCode(ds);
                psoDesc.HS = getShaderByteCode(hs);
                psoDesc.GS = getShaderByteCode(gs);
                psoDesc.PS = getShaderByteCode(ps);
                ID3D12RootSignature* pRootSignature;*/

            InputLayout inputLayout;

            std::vector<D3D12_INPUT_ELEMENT_DESC> d3dElementDescs(8);
            std::vector<std::string> semanticNames(8);

            initBlendDesc(gpsoDesc.blendDesc, d3d12psoDesc.BlendState);
            initResterizerDesc(gpsoDesc.rasterizerDesc, d3d12psoDesc.RasterizerState);
            initDepthStencilDesc(gpsoDesc.depthStencilDesc, d3d12psoDesc.DepthStencilState);
            initInputLayoutDesc(inputLayout, d3d12psoDesc.InputLayout, d3dElementDescs, semanticNames);

            d3d12psoDesc.SampleMask = UINT_MAX;
            d3d12psoDesc.PrimitiveTopologyType = getD3DPrimitiveTopologyType(gpsoDesc.primitiveTopology);

            uint32_t numRenderTargets = 0;
            for (uint32_t rtIndex = 0; rtIndex < gpsoDesc.colorAttachmentCount; ++rtIndex)
            {
                const auto format = gpsoDesc.colorAttachmentFormats[rtIndex];
                d3d12psoDesc.RTVFormats[rtIndex] = D3DUtils::GetDxgiResourceFormat(format);

                if (format == GpuResourceFormat::Unknown)
                    continue;

                numRenderTargets = rtIndex + 1;
            }

            d3d12psoDesc.NumRenderTargets = numRenderTargets;
            d3d12psoDesc.DSVFormat = D3DUtils::GetDxgiResourceFormat(gpsoDesc.depthStencilFormat);
            d3d12psoDesc.SampleDesc = {1, 0};///D3DUtils::GetSampleDesc(gpsoDesc.multisampleType);
            d3d12psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

            /*
            D3D12_GRAPHICS_PIPELINE_STATE_DESC
            D3DCall(DeviceContext::GetDevice()->CreateGraphicsPipelineState(
                getHeapProperties(resourceDesc.usage),
                D3D12_HEAP_FLAG_NONE,
                &d3dResourceDesc,
                defaultState_,
                pOptimizedClearValue,
                IID_PPV_ARGS(D3DResource_.put())));*/
        };
    }

}