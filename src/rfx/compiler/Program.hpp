#pragma once

namespace Rfx
{
    namespace Compiler
    {
        class CompileRequest;

        class Program final : public std::enable_shared_from_this<Program>
        {
        public:
            using SharedPtr = std::shared_ptr<Program>;
            using SharedConstPtr = std::shared_ptr<const Program>;

            bool GetShaderProgram(std::string& log);

        private:
        
            friend class CompileRequest;
        };
    }
}