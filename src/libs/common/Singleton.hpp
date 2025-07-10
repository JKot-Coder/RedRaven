#pragma once

#include "common/NonCopyableMovable.hpp"
namespace RR::Common
{
    template <class T>
    class Singleton : public NonCopyableMovable
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
        ~Singleton() = default;
    };
}