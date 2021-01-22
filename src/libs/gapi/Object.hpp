#pragma once

namespace OpenDemo
{
    namespace GAPI
    {
        template <typename T>
        class Resource;

        class Object : public std::enable_shared_from_this<Object>, private NonCopyable
        {
        public:
            enum class Type
            {
                Device,
                CommandQueue,
                CommandContext,
                CommandList,
                Fence,
                GpuResource,
                GpuResourceView,
                SwapChain,
            };

        public:
            using SharedPtr = std::shared_ptr<Object>;
            using SharedConstPtr = std::shared_ptr<const Object>;

            Object() = delete;
            virtual ~Object() = default;

            inline Type GetType() const { return type_; }
            inline U8String GetName() const { return name_; }

        private:
            Object(Type type, const U8String& name)
                : type_(type), name_(name)
            {
            }
            template <class T>
            friend class Resource;

        protected:
            Type type_;
            U8String name_;
        };
    }
}