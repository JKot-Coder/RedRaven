#pragma once

#include "gapi/Object.hpp"

namespace RR
{
    namespace GAPI
    {
        template <typename T, bool IsNamed = true>
        class Resource : public Object
        {
        public:
            virtual ~Resource() = default;

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

            template <typename = std::enable_if_t<IsNamed>>
            inline U8String GetName() const { return name_; }

        protected:
            template <std::enable_if_t<IsNamed, bool> = true>
            Resource(Type type, const U8String& name)
                : name_(name), Object(type)
            {
            }

            template <std::enable_if_t<IsNamed, bool> = false>
            Resource(Type type)
                : Object(type)
            {
            }

        private:
            // clang-format off
            struct monostate{};
            // clang-format on

            std::unique_ptr<T> privateImpl_ = nullptr;
            std::conditional_t<IsNamed, U8String, monostate> name_;
        };
    }
}