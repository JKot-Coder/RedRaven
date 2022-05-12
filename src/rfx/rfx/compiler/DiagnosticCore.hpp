#pragma once

#include "compiler/DiagnosticSink.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Diagnostics
        { // clang-format off
            #define DIAGNOSTIC(id, severity, name, messageFormat) static constexpr DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
            #include "compiler/DiagnosticDefinitions.hpp"
        } // clang-format on

        namespace LexerDiagnostics
        { // clang-format off
            #define DIAGNOSTIC(id, severity, name, messageFormat) static constexpr DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
            #include "compiler/LexerDiagnosticDefinitions.hpp"
        } // clang-format on
    }
}