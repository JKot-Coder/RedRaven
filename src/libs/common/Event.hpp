#pragma once

#include <functional>

namespace RR
{
    namespace Common
    {
        template <typename Signature>
        class Event;

        template <typename ReturnType, typename... Args>
        class Event<ReturnType(Args...)>
        {
        private:
            struct Delegate
            {
            public:
                ReturnType operator()(Args&&... args) const
                {
                    return (*callback_)(target_, std::forward<Args>(args)...);
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
                template <auto Callback>
                static Delegate Create()
                {
                    Delegate delegate;
                    delegate.target_ = nullptr;
                    delegate.callback_ = +[](void*, Args&&... args) -> ReturnType
                    {
                        return Callback(std::forward<Args>(args)...);
                    };

                    return delegate;
                }

                template <class Class, auto Callback>
                static Delegate Create(Class* target)
                {
                    Delegate delegate;
                    delegate.target_ = static_cast<void*>(target);
                    delegate.callback_ = +[](void* target, Args&&... args) -> ReturnType
                    {
                        return (static_cast<Class*>(target)->*Callback)(std::forward<Args>(args)...);
                    };

                    return delegate;
                }

            private:
                Delegate() = default;

            private:
                using StubFunction = ReturnType (*)(void*, Args&&...);

                void* target_;
                StubFunction callback_;
            };

        public:
            Event(std::size_t initialSize = 8U) { delegates_.reserve(initialSize); }

            template <auto Callback>
            void Subscribe()
            {
                const auto delegate = Delegate::template Create<Callback>();

                ASSERT_MSG(!isSubscribed(delegate), "Callback already subscribed");
                ASSERT_MSG(!protect_, "Callback subscribing is not allowed during dispatching");

                delegates_.push_back(delegate);
            }

            template <class Class, auto Callback>
            void Subscribe(Class* target)
            {
                const auto delegate = Delegate::template Create<Class, Callback>(target);

                ASSERT_MSG(!isSubscribed(delegate), "Callback already subscribed");
                ASSERT_MSG(!protect_, "Callback subscribing is not allowed during dispatching");

                delegates_.push_back(delegate);
            }

            template <auto Callback>
            void Unsubscribe()
            {
                ASSERT_MSG(!protect_, "Callback unsubscribing is not allowed during dispatching");

                unsubscribe(Delegate::template Create<Callback>());
            }

            template <class Class, auto Callback>
            void Unsubscribe(Class* target)
            {
                ASSERT_MSG(!protect_, "Callback unsubscribing is not allowed during dispatching");

                unsubscribe(Delegate::template Create<Class, Callback>(target));
            }

            template <auto Callback>
            bool IsSubscribed() const
            {
                return isSubscribed(Delegate::template Create<Callback>());
            }

            template <class Class, auto Callback>
            bool IsSubscribed() const
            {
                return isSubscribed(Delegate::template Create<Class, Callback>());
            }

            void Dispatch(Args... args) const
            {
                protect_ = true;

                for (const auto& delegate : delegates_)
                    delegate(std::forward<Args>(args)...);

                protect_ = false;
            }

        private:
            inline bool isSubscribed(const Delegate& delegate) const
            {
                return std::find(delegates_.begin(), delegates_.end(), delegate) != delegates_.end();
            }

            inline void unsubscribe(const Delegate& delegate)
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
