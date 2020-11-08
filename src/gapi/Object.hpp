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
            using ConstSharedPtrRef = const SharedPtr&;

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

            inline Type GetType() const { return _type; }
            inline U8String GetName() const { return _name; }

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
                : _type(type)
                , _name(name)
            {
            }

        protected:
            void* privateImpl_ = nullptr;
            Type _type;
            U8String _name;
        };

    }
}