#pragma once

#include "common/LinearAllocator.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"

namespace RR::Rfx
{
    struct CompileContext
    {
        CompileContext() : allocator(2048) { }

        DiagnosticSink sink;
        Common::LinearAllocator allocator;
    };
}