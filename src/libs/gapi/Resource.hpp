#pragma once

#include "EASTL/type_traits.h"
#include "common/Singleton.hpp"

namespace RR
{
    namespace GAPI
    {
        class ResourceDeletionQueue final : public Common::Singleton<ResourceDeletionQueue>
        {
        private:
            using DeleterFunc = void (*)(void*);

            struct Zombie
            {
                void* object;
                DeleterFunc deleter;
            };

            eastl::vector<Zombie> queue_;

        public:
            void Flush()
            {
                for (auto it = queue_.begin(); it != queue_.end(); ++it)
                    it->deleter(it->object);
                queue_.clear();
            }

            template <typename T>
            void PushDelete(T* object)
            {
                if (!object) return;
                queue_.push_back({object,
                                  [](void* ptr) { delete static_cast<T*>(ptr); }});
            }
        };

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
            virtual ~Resource() { ResourceDeletionQueue::Instance().PushDelete(privateImpl_.release()); }

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