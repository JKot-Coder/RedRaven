#pragma once

#include "parse_tools/DiagnosticSink.hpp"

namespace RR
{
    namespace ParseTools
    {
        namespace Diagnostics
        { // clang-format off
            #define DIAGNOSTIC(id, severity, name, messageFormat) static constexpr DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
            #include "parse_tools/DiagnosticDefinitions.hpp"
        } // clang-format on

        namespace LexerDiagnostics
        { // clang-format off
            #define DIAGNOSTIC(id, severity, name, messageFormat) static constexpr DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
            #include "parse_tools/LexerDiagnosticDefinitions.hpp"
        } // clang-format on
    }
}