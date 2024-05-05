#include "PipelineStateImpl.hpp"

#include "gapi/Framebuffer.hpp"
#include "gapi/PipelineState.hpp"

#include "gapi_dx12/DeviceContext.hpp"

#include "common/EnumClassOperators.hpp"

namespace RR
{
    namespace GAPI::DX12
    {
        struct DepthStencilDesc
        {
            enum class DepthAccess : uint8_t
            {
                None = 0,
                Read = 1,
                Write = 2,
                ReadWrite = Read | Write,
            }; // 2 bits
            ENUM_CLASS_FRIEND_BITWISE_OPS(DepthAccess);

            enum class ComparisonFunc : uint8_t
            {
                Never, // Comparison always fails
                Always, // Comparison always succeeds
                Less, // Passes if source is less than the destination
                Equal, // Passes if source is equal to the destination
                NotEqual, // Passes if source is not equal to the destination
                LessEqual, // Passes if source is less than or equal to the destination
                Greater, // Passes if source is greater than to the destination
                GreaterEqual, // Passes if source is greater than or equal to the destination
            }; // 3 bits

            enum class StencilOp : uint8_t
            {
                Keep, // Keep the stencil value
                Zero, // Set the stencil value to zero
                Replace, // Replace the stencil value with the reference value
                Increase, // Increase the stencil value by one, wrap if necessary
                IncreaseSaturate, /// Increase the stencil value by one, clamp if necessary
                Decrease, // Decrease the stencil value by one, wrap if necessary
                DecreaseSaturate, // Decrease the stencil value by one, clamp if necessary
                Invert //  Invert the stencil data (bitwise not)
            }; // 3 bits

            struct StencilDesc
            {
                ComparisonFunc func : 3; // Stencil comparison function
                StencilOp stencilFailOp : 3; // Stencil operation in case stencil test fails
                StencilOp depthFailOp : 3; /// Stencil operation in case stencil test passes but depth test fails
                StencilOp depthStencilPassOp : 3; // Stencil operation in case stencil and depth tests pass
            }; // 2 bytes

            DepthAccess depthAccess : 2;
            ComparisonFunc depthFunc : 3;
            bool stencilEnabled : 1;
            StencilDesc stencilFront;
            StencilDesc stencilBack;
            uint8_t stencilRef;
            uint8_t stencilReadMask;
            uint8_t stencilWriteMask;
        };

        struct GraphicPipelineStateDesc
        {
            enum class PrimitiveTopology : uint8_t
            {
                PointList,
                LineList,
                LineStrip,
                TriangleList,
                TriangleStrip,

                LineListAdj,
                LineStripAdj,
                TriangleListAdj,
                TriangleStripAdj,

                PathListControlPoint_1,
                PathListControlPoint_2,
                PathListControlPoint_3,
                PathListControlPoint_4,
                PathListControlPoint_5,
                PathListControlPoint_6,
                PathListControlPoint_7,
                PathListControlPoint_8,
                PathListControlPoint_9,
                PathListControlPoint_10,
                PathListControlPoint_11,
                PathListControlPoint_12,
                PathListControlPoint_13,
                PathListControlPoint_14,
                PathListControlPoint_15,
                PathListControlPoint_16,
                PathListControlPoint_17,
                PathListControlPoint_18,
                PathListControlPoint_19,
                PathListControlPoint_20,
                PathListControlPoint_21,
                PathListControlPoint_22,
                PathListControlPoint_23,
                PathListControlPoint_24,
                PathListControlPoint_25,
                PathListControlPoint_26,
                PathListControlPoint_27,
                PathListControlPoint_28,
                PathListControlPoint_29,
                PathListControlPoint_30,
                PathListControlPoint_31,
                PathListControlPoint_32
            };

            BlendDesc blendDesc;
            RasterizerDesc rasterizerDesc;
            DepthStencilDesc depthStencilDesc;
            FramebufferDesc framebufferDesc;
            PrimitiveTopology primitiveTopology;
        };

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
                    U8String semanticName;
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
            D3D12_BLEND getD3D12BlendFunc(BlendFunc func)
            {
                switch (func)
                {
                    case BlendFunc::Zero: return D3D12_BLEND_ZERO;
                    case BlendFunc::One: return D3D12_BLEND_ONE;
                    case BlendFunc::SrcColor: return D3D12_BLEND_SRC_COLOR;
                    case BlendFunc::OneMinusSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
                    case BlendFunc::DstColor: return D3D12_BLEND_DEST_COLOR;
                    case BlendFunc::OneMinusDstColor: return D3D12_BLEND_INV_DEST_COLOR;
                    case BlendFunc::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
                    case BlendFunc::OneMinusSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
                    case BlendFunc::DstAlpha: return D3D12_BLEND_DEST_ALPHA;
                    case BlendFunc::OneMinusDstAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
                    case BlendFunc::SrcAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
                    case BlendFunc::Src1Color: return D3D12_BLEND_INV_SRC1_COLOR;
                    case BlendFunc::OneMinusSrc1Color: return D3D12_BLEND_INV_SRC1_COLOR;
                    case BlendFunc::Src1Alpha: return D3D12_BLEND_SRC1_ALPHA;
                    case BlendFunc::OneMinusSrc1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
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

            D3D12_PRIMITIVE_TOPOLOGY_TYPE getD3DPrimitiveTopologyType(GraphicPipelineStateDesc::PrimitiveTopology topology)
            {
                switch (topology)
                {
                    case GraphicPipelineStateDesc::PrimitiveTopology::PointList:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

                    case GraphicPipelineStateDesc::PrimitiveTopology::LineList:
                    case GraphicPipelineStateDesc::PrimitiveTopology::LineStrip:
                    case GraphicPipelineStateDesc::PrimitiveTopology::LineListAdj:
                    case GraphicPipelineStateDesc::PrimitiveTopology::LineStripAdj:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

                    case GraphicPipelineStateDesc::PrimitiveTopology::TriangleList:
                    case GraphicPipelineStateDesc::PrimitiveTopology::TriangleStrip:
                    case GraphicPipelineStateDesc::PrimitiveTopology::TriangleListAdj:
                    case GraphicPipelineStateDesc::PrimitiveTopology::TriangleStripAdj:
                        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_1:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_2:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_3:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_4:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_5:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_6:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_7:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_8:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_9:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_10:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_11:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_12:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_13:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_14:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_15:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_16:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_17:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_18:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_19:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_20:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_21:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_22:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_23:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_24:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_25:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_26:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_27:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_28:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_29:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_30:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_31:
                    case GraphicPipelineStateDesc::PrimitiveTopology::PathListControlPoint_32:
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

                static_assert(std::size(BlendDesc {}.rtBlend) == std::size(D3D12_BLEND_DESC {}.RenderTarget));

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
                    d3dRtDesc.SrcBlend = getD3D12BlendFunc(rtDesc.srcRgbFunc);
                    d3dRtDesc.DestBlend = getD3D12BlendFunc(rtDesc.dstRgbFunc);
                    d3dRtDesc.BlendOpAlpha = getD3D12BlendOp(rtDesc.alphaBlendOp);
                    d3dRtDesc.SrcBlendAlpha = getD3D12BlendFunc(rtDesc.srcAlphaFunc);
                    d3dRtDesc.DestBlendAlpha = getD3D12BlendFunc(rtDesc.dstAlphaFunc);

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
                                     std::vector<U8String>& semanticNames)
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
            std::vector<U8String> semanticNames(8);

            initBlendDesc(gpsoDesc.blendDesc, d3d12psoDesc.BlendState);
            initResterizerDesc(gpsoDesc.rasterizerDesc, d3d12psoDesc.RasterizerState);
            initDepthStencilDesc(gpsoDesc.depthStencilDesc, d3d12psoDesc.DepthStencilState);
            initInputLayoutDesc(inputLayout, d3d12psoDesc.InputLayout, d3dElementDescs, semanticNames);

            d3d12psoDesc.SampleMask = UINT_MAX;
            d3d12psoDesc.PrimitiveTopologyType = getD3DPrimitiveTopologyType(gpsoDesc.primitiveTopology);

            uint32_t numRenderTargets = 0;
            for (uint32_t rtIndex = 0; rtIndex < gpsoDesc.framebufferDesc.renderTargetViews.size(); ++rtIndex)
            {
                const auto format = gpsoDesc.framebufferDesc.renderTargetViews[rtIndex]->GetDescription().format;
                d3d12psoDesc.RTVFormats[rtIndex] = D3DUtils::GetDxgiResourceFormat(format);

                if (format == GpuResourceFormat::Unknown)
                    continue;

                numRenderTargets = rtIndex + 1;
            }

            d3d12psoDesc.NumRenderTargets = numRenderTargets;
            d3d12psoDesc.DSVFormat = D3DUtils::GetDxgiResourceFormat(gpsoDesc.framebufferDesc.depthStencilView->GetDescription().format);
            d3d12psoDesc.SampleDesc = D3DUtils::GetSampleDesc(gpsoDesc.framebufferDesc.multisampleType);
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