#include "Session.hpp"

#include "compiler/CompileRequest.hpp"

#include <slang.h>

namespace Rfx
{
    namespace Compiler
    {
        Session::Session()
        {
           // slang::createGlobalSession(session_.writeRef());
            ASSERT(session_);            
        }

        CompileRequest::SharedPtr Session::CreateCompileRequest()
        {
            return CompileRequest::SharedPtr(new CompileRequest(session_));
        }
    }
}