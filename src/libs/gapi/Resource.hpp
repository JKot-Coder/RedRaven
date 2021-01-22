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
            virtual ~Resource() {};

            inline T* GetPrivateImpl()
            {
                return privateImpl_.get();
            }

            inline const T* GetPrivateImpl() const
            {
                return privateImpl_.get();
            }

            template <typename T1>
            inline T1* GetPrivateImpl()
            {
                ASSERT(dynamic_cast<T1*>(privateImpl_.get()));
                return static_cast<T1*>(privateImpl_.get());
            }

            template <typename T1>
            inline const T1* GetPrivateImpl() const
            {
                ASSERT(dynamic_cast<T1*>(privateImpl_.get()));
                return static_cast<const T1*>(privateImpl_.get());
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