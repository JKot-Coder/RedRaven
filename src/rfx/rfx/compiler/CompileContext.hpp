#pragma once

#include "common/LinearAllocator.hpp"
#include "rfx/core/SourceManager.hpp"
#include "rfx/compiler/DiagnosticCore.hpp"

namespace RR::Rfx
{
    struct CompileContext
    {
        CompileContext(bool onlyRelativePaths) : sink(onlyRelativePaths), allocator(2048), onlyRelativePaths(onlyRelativePaths) { }

        DiagnosticSink sink;
        Common::LinearAllocator allocator;
        SourceManager sourceManager;

        bool onlyRelativePaths = false;
    };
}