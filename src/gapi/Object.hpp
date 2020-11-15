#pragma once

namespace OpenDemo
{
    namespace Render
    {

        class Object : public std::enable_shared_from_this<Object>, private NonCopyable
        {
        public:
            using SharedPtr = std::shared_ptr<Object>;
            using SharedConstPtr = std::shared_ptr<const Object>;

            enum class Type
            {
                RenderQueue,
                CommandContext,
                CommandList,                
                Resource,                
                ResourceView
            };

            Object() = delete;

            virtual ~Object() = default;

            inline Type GetType() const { return type_; }
            inline U8String GetName() const { return name_; }

            template <typename T>
            T GetPrivateImpl()
            {
                return reinterpret_cast<T>(privateImpl_);
            }

            template <typename T>
            const T* GetPrivateImpl() const
            {
                return reinterpret_cast<T*>(privateImpl_);
            }

            template <typename T>
            void SetPrivateImpl(T* privateImpl) { privateImpl_ = privateImpl; }

        protected:
            Object(Type type, const U8String& name)
                : type_(type)
                , name_(name)
            {
            }

        protected:
            void* privateImpl_ = nullptr;
            Type type_;
            U8String name_;
        };

    }
}