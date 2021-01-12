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
                static T instance;
                return instance;
            }

        protected:
            Singleton() = default;
        };
    }
}