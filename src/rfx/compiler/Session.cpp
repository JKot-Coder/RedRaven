#include "Session.hpp"

#include "compiler/CompileRequest.hpp"

namespace Rfx
{
    namespace Compiler
    {
        Session::Session()
        {
            // slang::createGlobalSession(session_.writeRef());
            //  ASSERT(session_);
        }

        CompileRequest::SharedPtr Session::CreateCompileRequest()
        {
            return nullptr;

            //   return CompileRequest::SharedPtr(new CompileRequest(session_));
        }
    }
}