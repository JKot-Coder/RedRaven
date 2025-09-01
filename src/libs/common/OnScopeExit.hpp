#pragma once

#define ON_SCOPE_EXIT_NAME2(y) scopeExit_##y
#define ON_SCOPE_EXIT_NAME(y) ON_SCOPE_EXIT_NAME2(y)
#define ON_SCOPE_EXIT(...) [[maybe_unused]] const RR::Common::OnScopeExit ON_SCOPE_EXIT_NAME(__COUNTER__)(__VA_ARGS__);

namespace RR
{
    namespace Common
    {
        template <typename F>
        class OnScopeExit final : public Common::NonCopyableMovable
        {
        public:
            explicit OnScopeExit(F&& function) : function_(function) { }
            ~OnScopeExit() { function_(); }

        private:
            F function_;
        };
    }
}