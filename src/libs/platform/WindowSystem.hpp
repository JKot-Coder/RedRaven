#pragma once

#include "platform/Window.hpp"

#include "common/Singleton.hpp"

namespace RR
{
    namespace Platform
    {
#ifdef OS_WINDOWS
        constexpr wchar_t WINDOW_CLASS_NAME[] = L"RedRevenWndClass";
#endif

        class WindowSystem final : public Singleton<WindowSystem>
        {
        public:
            WindowSystem() = default;
            ~WindowSystem();

            void Init();

            std::shared_ptr<Window> Create(const Window::Description& description) const;
            void PoolEvents() const;

        private:
            bool isInited_ = false;
        };
    }
}