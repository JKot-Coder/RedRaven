#pragma once

#include "gapi/Object.hpp"

namespace OpenDemo
{
    namespace GAPI
    {
        template <typename T>
        class Resource : public Object
        {
        public:
            inline T* GetPrivateImpl()
            {
                return privateImpl_.get();
            }

            inline const T* GetPrivateImpl() const
            {
                return privateImpl_.get();
            }

            inline void SetPrivateImpl(T* impl)
            {
                privateImpl_.reset(impl);
            }

        protected:
            Resource(Type type, const U8String& name)
                : Object(type, name)
            {
            }

        protected:
            std::unique_ptr<T> privateImpl_ = nullptr;
        };
    }
}