#pragma once

namespace slang
{
    struct ICompileRequest;
    struct IGlobalSession;
}

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
            CompileRequest(const ::Slang::ComPtr<slang::IGlobalSession>& globalSesion);

        private:
            void clearModules();

        private:
            ::Slang::ComPtr<slang::ISession> session_;
            std::list<slang::IComponentType*> components_;

            friend class Session;
        };
    }
}