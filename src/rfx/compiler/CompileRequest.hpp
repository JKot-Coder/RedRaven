#pragma once

namespace Rfx
{
    enum class CompileTarget : uint32_t;

    namespace Compiler
    {
        class Session;
        class Program;

        class CompileRequest final : public std::enable_shared_from_this<CompileRequest>
        {
        public:
            struct Description final
            {
                CompileTarget target;
            };

        public:
            using SharedPtr = std::shared_ptr<CompileRequest>;
            using SharedConstPtr = std::shared_ptr<const CompileRequest>;

            ~CompileRequest();

            bool LoadModule(const std::string& name, std::string& log);
            bool AddEntryPoint();

            std::shared_ptr<Program> Compile(std::string& log);

        private:
            void clearModules();


            friend class Session;
        };
    }
}