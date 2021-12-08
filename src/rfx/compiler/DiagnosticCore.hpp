#pragma once

#include "compiler/DiagnosticSink.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            namespace LexerDiagnostics
            { // clang-format off
            #define DIAGNOSTIC(id, severity, name, messageFormat) extern const DiagnosticInfo name;
            #include "compiler/LexerDiagnosticDefinitions.hpp"
        } // clang-format on
        }
    }
}