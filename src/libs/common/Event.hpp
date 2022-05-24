#pragma once

#include <functional>

namespace RR
{
    namespace Common
    {
        template <typename... Args>
        class Event
        {
        private:
            // It is possible to implement optionally interrupted events.
            // A good start is to declare ReturnType as std::conditional
            using ReturnType = void;

            template <class Class>
            using ClassMemberCallback = ReturnType (Class::*)(Args...);
            using CallbackFunction = ReturnType (*)(Args...);

            struct Delegate
            {
            public:
                using StubFunction = ReturnType (*)(void*, Args...);

                ReturnType operator()(Args... args) const
                {
                    return (*callback_)(target_, args...);
                }

                bool operator==(const Delegate& other) const
                {
                    return (callback_ == other.callback_) && (target_ == other.target_);
                }

                bool operator!=(const Delegate& other) const
                {
                    return !(*this == other);
                }

            public:
                template <CallbackFunction Callback>
                static Delegate Create()
                {
                    Delegate delegate;
                    delegate.target_ = nullptr;
                    delegate.callback_ = &stabFunction<Callback>;

                    return delegate;
                }

                template <class Class, ClassMemberCallback<Class> Callback>
                static Delegate Create(Class* target)
                {
                    Delegate delegate;
                    delegate.target_ = static_cast<void*>(target);
                    delegate.callback_ = &stabFunction<Class, Callback>;

                    return delegate;
                }

            private:
                Delegate() = default;

            private:
                template <CallbackFunction Callback>
                static void stabFunction(void*, Args... args)
                {
                    return Callback(args...);
                }

                template <class Class, ClassMemberCallback<Class> Callback>
                static void stabFunction(void* target, Args... args)
                {
                    return (static_cast<Class*>(target)->*Callback)(args...);
                }

            private:
                void* target_;
                StubFunction callback_;
            };

        public:
            template <CallbackFunction Callback>
            void Register()
            {
                const auto delegate = Delegate::Create<Callback>();

                ASSERT_MSG(!isRegistered(delegate), "Callback already registered");
                ASSERT_MSG(!protect_, "Callback registration is not allowed during dispatching");

                delegates_.push_back(Delegate::Create<Callback>());
            }

            template <class Class, ClassMemberCallback<Class> Callback>
            void Register(Class* target)
            {
                const auto delegate = Delegate::Create<Class, Callback>(target);

                ASSERT_MSG(!isRegistered(delegate), "Callback already registered");
                ASSERT_MSG(!protect_, "Callback registration is not allowed during dispatching");

                delegates_.push_back(delegate);
            }

            template <CallbackFunction Callback>
            void Unregister()
            {
                ASSERT_MSG(!protect_, "Callback unregistration is not allowed during dispatching");

                unregister(Delegate::Create<Callback>());
            }

            template <class Class, ClassMemberCallback<Class> Callback>
            void Unregister(Class* target)
            {
                ASSERT_MSG(!protect_, "Callback unregistration is not allowed during dispatching");

                unregister(Delegate::Create<Class, Callback>(target));
            }

            template <CallbackFunction Callback>
            bool IsRegistered() const
            {
                return isRegistered(Delegate::Create<Callback>());
            }

            template <class Class, ClassMemberCallback<Class> Callback>
            bool IsRegistered() const
            {
                return isRegistered(Delegate::Create<Class, Callback>());
            }

            void Dispatch(Args... args) const
            {
                protect_ = true;

                for (const auto& delegate : delegates_)
                    delegate(args...);

                protect_ = false;
            }

        private:
            inline bool isRegistered(const Delegate& delegate) const
            {
                return std::find(delegates_.begin(), delegates_.end(), delegate) != delegates_.end();
            }

            inline void unregister(const Delegate& delegate)
            {
                delegates_.erase(
                    std::remove(delegates_.begin(), delegates_.end(), delegate),
                    delegates_.end());
            }

        private:
            std::vector<Delegate> delegates_;
            mutable bool protect_ = false;
        };
    }
}
