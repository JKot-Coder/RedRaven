#include "platform/Toolkit.hpp"

namespace RR
{
    namespace Platform
    {
        class GlfwToolkit final : public IToolkit
        {
        public:
            GlfwToolkit() = default;
            ~GlfwToolkit();

            void Init();
            std::vector<Monitor> GetMonitors() const override;

            void PoolEvents() const override;

            // CreatePlatformWindow to avoid name collision with CreateWindow define from Windows.h
            std::shared_ptr<Window> CreatePlatformWindow(const Window::Description& description) const override;

        private:
            bool isInited_ = false;
        };
    }
}