#pragma once

namespace Rfx
{
    namespace Compiler
    {
        class CompileRequest;

        class Session final : public std::enable_shared_from_this<Session>
        {
        public:
            using SharedPtr = std::shared_ptr<Session>;
            using SharedConstPtr = std::shared_ptr<const Session>;

            Session();

            std::shared_ptr<CompileRequest> CreateCompileRequest();
        };
    }
}