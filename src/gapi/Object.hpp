#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        class Object : public std::enable_shared_from_this<Object>, private NonCopyable
        {
        public:
            using SharedPtr = std::shared_ptr<Object>;
            using SharedConstPtr = std::shared_ptr<const Object>;

            enum class Type
            {
                CommandQueue,
                CommandContext,
                CommandList,
                Fence,
                Resource,
                ResourceView,
                SwapChain,
            };

            Object() = delete;

            virtual ~Object() = default;

            inline Type GetType() const { return type_; }
            inline U8String GetName() const { return name_; }

            inline bool IsPrivateImplNull() const { return privateImpl_ == nullptr; };

        protected:
            Object(Type type, const U8String& name)
                : type_(type), name_(name)
            {
            }

        protected:
            void* privateImpl_ = nullptr;
            Type type_;
            U8String name_;
        };

        class PrivateImplementedObject : public Object
        {
        public:
            template <typename T>
            inline T* GetPrivateImpl()
            {
                return static_cast<T*>(privateImpl_);
            }

            template <typename T>
            const T* GetPrivateImpl() const
            {
                return static_cast<T*>(privateImpl_);
            }

            template <typename T>
            inline void SetPrivateImpl(T* impl)
            {
                privateImpl_ = impl;
            }

        protected:
            PrivateImplementedObject(Type type, const U8String& name) : Object(type, name) {};
        };

        template <typename T>
        class InterfaceWrapObject : public Object
        {
        public:
            inline T* GetInterface()
            {
                return static_cast<T*>(privateImpl_);
            }

            const T* GetInterface() const
            {
                return static_cast<T*>(privateImpl_);
            }

            inline void SetInterface(T* impl)
            {
                privateImpl_ = impl;
            }

        protected:
            InterfaceWrapObject(Type type, const U8String& name) : Object(type, name) {};
        };
    }
}