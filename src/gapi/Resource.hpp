#pragma once

namespace OpenDemo
{
    namespace Render
    {

        class Resource : public std::enable_shared_from_this<Resource>
        {
        public:
            using SharedPtr = std::shared_ptr<Resource>;
            using SharedConstPtr = std::shared_ptr<const Resource>;
            using ConstSharedPtrRef = const SharedPtr&;

            enum class Type
            {
                Buffer,
                CommandList
            };

            Resource() = delete;
            Resource(const Resource&) = delete;
            Resource& operator=(const Resource&) = delete;

            virtual ~Resource() = default;

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
            Resource(Type type, const U8String& name)
                : _type(type)
                , _name(name)
            {
            }

        private:
            void* privateImpl_;
            Type _type;
            U8String _name;
        };

    }
}