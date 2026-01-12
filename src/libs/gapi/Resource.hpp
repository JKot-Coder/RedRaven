#pragma once

#include "EASTL/type_traits.h"

namespace RR
{
    namespace GAPI
    {
        template <typename T, bool IsNamed = true>
        class Resource : public Common::NonCopyable
        {
        public:
            enum class Type
            {
                CommandEncoder,
                CommandList,
                CommandQueue,
                Device,
                Fence,
                Framebuffer,
                GpuResource,
                GpuResourceView,
                MemoryAllocation,
                SwapChain,
                Shader,
                PipelineState,
                BindingLayout,
                BindingSet,
            };

        public:
            virtual ~Resource() = default;

            T* GetPrivateImpl() { return privateImpl_.get(); }
            const T* GetPrivateImpl() const { return privateImpl_.get(); }

            template <typename T1>
            T1* GetPrivateImpl()
            {
                ASSERT(dynamic_cast<T1*>(privateImpl_.get()));
                return static_cast<T1*>(privateImpl_.get());
            }

            template <typename T1>
            const T1* GetPrivateImpl() const
            {
                ASSERT(dynamic_cast<T1*>(privateImpl_.get()));
                return static_cast<const T1*>(privateImpl_.get());
            }
            void SetPrivateImpl(T* impl) { privateImpl_.reset(impl); }

            template <bool isNamed = IsNamed, typename = eastl::enable_if_t<isNamed>>
            const std::string& GetName() const { return name_; }

        protected:
            Resource(Resource&& other) noexcept : type_(other.type_), privateImpl_(eastl::move(other.privateImpl_)), name_(eastl::move(other.name_)) { };

            template <bool isNamed = IsNamed, typename = eastl::enable_if_t<isNamed>>
            Resource(Type type, const std::string& name) : type_(type), name_(name) { }

            template <bool isNamed = IsNamed, typename = eastl::enable_if_t<!isNamed>>
            explicit Resource(Type type) : type_(type) { }

        private:
            // clang-format off
            struct monostate{};
            // clang-format on

            Type type_;
            eastl::unique_ptr<T> privateImpl_ = nullptr;
            eastl::conditional_t<IsNamed, std::string, monostate> name_;
        };
    }
}