#pragma once

#include "gapi/GpuResource.hpp"
#include "GraphicsTypes.h"

namespace DL = ::Diligent;

namespace RR::GAPI::Diligent
{
    DL::TEXTURE_FORMAT GetDLTextureFormat(GpuResourceFormat format);
}