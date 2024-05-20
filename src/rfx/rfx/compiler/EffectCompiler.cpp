#include "EffectCompiler.hpp"

#include "compiler/CompileContext.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/compiler/RSONValue.hpp"

#include "gapi/PipelineState.hpp"

#include "common/Result.hpp"
#include "common/hash/Default.hpp"

namespace RR::Rfx
{
    using RResult = Common::RResult;
    using namespace Common::Hash::Default;

    constexpr inline HashType Hash(UnownedStringSlice slice) { return Common::Hash::Default::Hash(slice.data(), slice.size()); }

    EffectCompiler::EffectCompiler(const std::shared_ptr<CompileContext>& context) : context_(context) { }
    EffectCompiler::~EffectCompiler() { }

    DiagnosticSink& EffectCompiler::getSink() const { return context_->sink; }
    RResult EffectCompiler::throwInvalidKey(UnownedStringSlice key, RSONValue value) const
    {
        getSink().Diagnose(Diagnostics::unknownKeyValue, key, value.asString());
        return RResult::NotFound;
    }

    RResult EffectCompiler::throwInvalidKey(UnownedStringSlice key, UnownedStringSlice value) const
    {
        getSink().Diagnose(Diagnostics::unknownKeyValue, key, value);
        return RResult::NotFound;
    }

    void EffectCompiler::Compile(RSONValue rson)
    {
        const auto& effect = rson["Effect"];

        if (!effect.isValid())
            return;

        const auto& tehniques = effect["Tehniques"];

        if (tehniques.empty())
            return;

        for (const auto& tehniqueRef : tehniques)
        {
            if (!tehniqueRef.second.isReference())
                return;

            const auto& tehnique = tehniqueRef.second.getReferenced();
            std::ignore = parseTehnique(tehnique);
        }
    }

    RResult EffectCompiler::parseTehnique(RSONValue tehnique) const
    {
        const auto& passes = tehnique["Passes"];

        for (const auto& pass : passes)
            RR_RETURN_ON_FAIL(parsePass(pass.second));

        return RResult::Ok;
    }

    RResult EffectCompiler::parsePass(RSONValue pass) const
    {
        const auto& renderState = pass["RenderState"];
        if (!renderState.isObject())
            return RResult::Fail;

        GAPI::RasterizerDesc rasterizerDesc;
        RR_RETURN_ON_FAIL(parseRenderState(renderState, rasterizerDesc));

        return RResult::Ok;
    }

    RResult EffectCompiler::parseRasterizerDesc(UnownedStringSlice key, RSONValue value, GAPI::RasterizerDesc& rasterizerDesk) const
    {
        const auto getFillMode = [&rasterizerDesk, key, this](UnownedStringSlice value) {
            switch (Hash(value))
            {
                case "Solid"_h: rasterizerDesk.fillMode = GAPI::FillMode::Solid; break;
                case "Wireframe"_h: rasterizerDesk.fillMode = GAPI::FillMode::Wireframe; break;
                default: return throwInvalidKey(key, value);
            }
            return RResult::Ok;
        };

        const auto getCullMode = [&rasterizerDesk, key, this](UnownedStringSlice value) {
            switch (Hash(value))
            {
                case "None"_h: rasterizerDesk.cullMode = GAPI::CullMode::None; break;
                case "Front"_h: rasterizerDesk.cullMode = GAPI::CullMode::Front; break;
                case "Back"_h: rasterizerDesk.cullMode = GAPI::CullMode::Back; break;
                default: return throwInvalidKey(key, value);
            }
            return RResult::Ok;
        };

        switch (Hash(key))
        {
            case "FillMode"_h: RR_RETURN_ON_FAIL(getFillMode(value.getString("Solid"))); break;
            case "CullMode"_h: RR_RETURN_ON_FAIL(getCullMode(value.getString("Back"))); break;
            case "SlopeScaledDepthBias"_h: rasterizerDesk.slopeScaledDepthBias = static_cast<float>(value.getFloat(0.0)); break;
            case "DepthBias"_h: rasterizerDesk.depthBias = static_cast<float>(value.getFloat(0.0)); break;
            case "IndependentBlend"_h: rasterizerDesk.independentBlendEnabled = value.getBool(false); break;
            case "AlphaToCoverage"_h: rasterizerDesk.alphaToCoverageEnabled = value.getBool(false); break;
            case "BlendStates"_h:
            {
                if (!value.isArray())
                    return RResult::Fail;
                for (const auto& blendState : value)
                {
                    if (!blendState.second.isObject())
                        return RResult::Fail;

                    GAPI::RTBlendStateDesc blendDesk;

                    for (const auto& keyValue : blendState.second)
                        RR_RETURN_ON_FAIL(parseBlendStateDesc(keyValue.first, keyValue.second, blendDesk));
                }
            }
            break;

            default: return throwInvalidKey(key, value);
        }

        return RResult::Ok;
    }

    RResult EffectCompiler::parseBlendStateDesc(UnownedStringSlice key, RSONValue value, GAPI::RTBlendStateDesc& blendDesk) const
    {
        switch (Hash(key))
        {
            case "BlendEnable"_h: blendDesk.blendEnabled = value.getBool(false); break;
            case "BlendOp"_h: blendDesk.rgbBlendOp = value.getEnum(GAPI::BlendOp::Add); break;
            case "SrcBlendFunc"_h: blendDesk.srcRgbFunc = value.getEnum(GAPI::BlendFunc::One); break;
            case "DstBlendFunc"_h: blendDesk.dstRgbFunc = value.getEnum(GAPI::BlendFunc::One); break;
            case "BlendOpAlpha"_h: blendDesk.alphaBlendOp = value.getEnum(GAPI::BlendOp::Add); break;
            case "SrcBlendFuncAlpha"_h: blendDesk.srcAlphaFunc = value.getEnum(GAPI::BlendFunc::One); break;
            case "DstBlendFuncAlpha"_h: blendDesk.dstAlphaFunc = value.getEnum(GAPI::BlendFunc::One); break;
            case "WriteMask"_h: blendDesk.writeMask = value.getEnum(GAPI::WriteMask::All); break;
            default: return throwInvalidKey(key, value);
        }

        return RResult::Ok;
    }

    RResult EffectCompiler::parseRenderState(RSONValue renderState, GAPI::RasterizerDesc& rasterizerDesc) const
    {
        for (const auto& keyValue : renderState)
            RR_RETURN_ON_FAIL(parseRasterizerDesc(keyValue.first, keyValue.second, rasterizerDesc));

        return RResult::Ok;
    }
}