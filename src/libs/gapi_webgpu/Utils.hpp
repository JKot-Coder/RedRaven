#pragma once

#include "webgpu/webgpu.hpp"

namespace RR::GAPI
{
    enum class GpuResourceFormat : uint32_t;
}

namespace RR::GAPI::WebGPU
{
    wgpu::TextureFormat GetWGPUFormat(GpuResourceFormat format);
}