#pragma once

#include "common/LinearAllocator.hpp"
#include "parse_tools/DiagnosticCore.hpp"
#include "parse_tools/core/SourceManager.hpp"

namespace RR::ParseTools
{
    struct CompileContext
    {
        CompileContext(bool onlyRelativePaths)
            : sink(onlyRelativePaths), allocator(2048), onlyRelativePaths(onlyRelativePaths)
        {
        }

        DiagnosticSink sink;
        Common::LinearAllocator<> allocator;

        bool onlyRelativePaths = false;
    };
}