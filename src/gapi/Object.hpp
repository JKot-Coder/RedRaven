#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        template <typename T>
        class PrivateImplementedObject;

        class Object : public std::enable_shared_from_this<Object>, private NonCopyable
        {
        public:
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

        public:
            using SharedPtr = std::shared_ptr<Object>;
            using SharedConstPtr = std::shared_ptr<const Object>;

            Object() = delete;
            virtual ~Object() { }

            inline Type GetType() const { return type_; }
            inline U8String GetName() const { return name_; }

        private:
            Object(Type type, const U8String& name)
                : type_(type), name_(name)
            {
            }
            template <class T>
            friend class PrivateImplementedObject;

        protected:
            Type type_;
            U8String name_;
        };

        template <typename T>
        class PrivateImplementedObject : public Object
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
            PrivateImplementedObject(Type type, const U8String& name)
                : Object(type, name)
            {
            }

        protected:
            std::unique_ptr<T> privateImpl_ = nullptr;
        };

    }
}