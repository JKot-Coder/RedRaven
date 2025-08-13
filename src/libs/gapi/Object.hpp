#pragma once

namespace RR::GAPI
{
    class Object : public eastl::enable_shared_from_this<Object>, public Common::NonCopyable
    {
    public:
        enum class Type
        {
            CommandContext,
            CommandList,
            CommandQueue,
            Device,
            Fence,
            Framebuffer,
            GpuResource,
            GpuResourceView,
            MemoryAllocation,
            SwapChain,
        };

    public:
        using SharedPtr = eastl::shared_ptr<Object>;
        using SharedConstPtr = eastl::shared_ptr<const Object>;

        Object() = delete;
        virtual ~Object() = default;

        inline Type GetType() const { return type_; }

    private:
        Object(Type type) : type_(type) { }

        template <class T, bool IsNamed>
        friend class Resource;

    protected:
        Type type_;
    };

}