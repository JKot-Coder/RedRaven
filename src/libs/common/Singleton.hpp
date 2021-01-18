#pragma once

namespace OpenDemo
{
    namespace Common
    {
        template <class T>
        class Singleton : private NonCopyable, NonMovable
        {
        public:
            static T& Instance()
            {
                static_assert(std::is_default_constructible<T>::value, "T should be default constructible");
                static T instance;
                return instance;
            }

        protected:
            Singleton() = default;
        };
    }
}