#pragma once

#include "gapi/Shader.hpp"

#include "RefCntAutoPtr.hpp"

namespace Diligent
{
    struct IRenderDevice;
    struct IShader;
}
namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    class ShaderImpl final : public IShader
    {
    public:
        ShaderImpl() = default;
        ~ShaderImpl() override;

        void Init(DL::IRenderDevice* device, Shader& resource);

        DL::IShader* GetShader() const { return shader.RawPtr(); }
    private:
        DL::RefCntAutoPtr<DL::IShader> shader;
    };
}