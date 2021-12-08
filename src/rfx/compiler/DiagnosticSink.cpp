#include "DiagnosticSink.hpp"

#include "compiler/Signal.hpp"

namespace RR
{
    namespace Rfx
    {
        namespace Compiler
        {
            U8String GetSeverityName(Severity severity)
            {
                switch (severity)
                {
                    case Severity::Note:
                        return "note";
                    case Severity::Warning:
                        return "warning";
                    case Severity::Error:
                        return "error";
                    case Severity::Fatal:
                        return "fatal error";
                    case Severity::Internal:
                        return "internal error";
                    default:
                        return "unknown error";
                }
            }

            void DiagnosticSink::diagnoseImpl(const DiagnosticInfo& info, const U8String& formattedMessage)
            {
                if (info.severity >= Severity::Error)
                    errorCount_++;

                (void)formattedMessage;
                /*
            if (writer)
            {
                writer->write(formattedMessage.begin(), formattedMessage.getLength());
            }
            else
            {
                outputBuffer.append(formattedMessage);
            }
            */
                 
                /*
            if (m_parentSink)
            {
                m_parentSink->diagnoseImpl(info, formattedMessage);
            }
            */

                if (info.severity >= Severity::Fatal)
                {
                    // TODO: figure out a better policy for aborting compilation
                    RFX_ABORT_COMPILATION("");
                }
            }

            U8String DiagnosticSink::formatDiagnostic(/*const HumaneSourceLocation& humaneLoc, */ Diagnostic const& diagnostic /*, DiagnosticSink::Flags flags, StringBuilder& outBuilder*/)
            {
                /*  if (flags & DiagnosticSink::Flag::HumaneLoc)
                {
                    outBuilder << humaneLoc.pathInfo.foundPath;
                    outBuilder << "(";
                    outBuilder << Int32(humaneLoc.line);
                    outBuilder << "): ";
                }*/

                U8String format = (diagnostic.errorID >= 0) ? "{0} {1}: {2}\n" : "{0}: {2}\n";

                return fmt::format(format,
                                   GetSeverityName(diagnostic.severity),
                                   diagnostic.errorID,
                                   diagnostic.message);
            }
        }
    }
}