#pragma once

namespace RR
{
    namespace GAPI
    {
        class Object : public std::enable_shared_from_this<Object>, private NonCopyable
        {
        public:
            enum class Type
            {
                CommandContext,
                CommandList,
                CommandQueue,
                Device,
                Fence,
                GpuResource,
                GpuResourceView,
                MemoryAllocation,
                SwapChain,
            };

        public:
            using SharedPtr = std::shared_ptr<Object>;
            using SharedConstPtr = std::shared_ptr<const Object>;

            Object() = delete;
            virtual ~Object() = default;

            inline Type GetType() const { return type_; }

        private:
            Object(Type type)
                : type_(type)
            {
            }

            template <class T, bool IsNamed>
            friend class Resource;

        protected:
            Type type_;
        };
    }
}