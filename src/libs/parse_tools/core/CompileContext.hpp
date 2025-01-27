#pragma once

#include "common/LinearAllocator.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"
#include "rfx/core/SourceManager.hpp"

namespace RR::Rfx
{
    struct CompileContext
    {
        CompileContext(bool onlyRelativePaths)
            : sink(onlyRelativePaths), allocator(2048), onlyRelativePaths(onlyRelativePaths)
        {
        }

        DiagnosticSink sink;
        Common::LinearAllocator allocator;

        bool onlyRelativePaths = false;
    };
}