#include "DiagnosticCore.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            namespace LexerDiagnostics
            { // clang-format off
            #define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
            #include "compiler/LexerDiagnosticDefinitions.hpp"
            #undef DIAGNOSTIC            
        } // clang-format on
        }
    }
}