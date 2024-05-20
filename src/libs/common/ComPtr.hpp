#pragma once

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

#include <Guiddef.h>
#include <unknwn.h>

namespace RR::Common
{
    template <typename T>
    struct ComPtr;

    namespace impl
    {
        template <typename T, typename Enable = void>
        struct abi
        {
            using type = T;
        };

        template <typename T>
        struct abi<T, std::enable_if_t<std::is_enum_v<T>>>
        {
            using type = std::underlying_type_t<T>;
        };

        template <typename T>
        using abi_t = typename abi<T>::type;

        struct take_ownership_from_abi_t
        {
        };
        inline constexpr take_ownership_from_abi_t take_ownership_from_abi {};

    }

    template <typename T>
    struct ComPtr
    {
        using type = impl::abi_t<T>;

        ComPtr(std::nullptr_t = nullptr) noexcept { }
        ComPtr(void* ptr, impl::take_ownership_from_abi_t) noexcept : ptr_(static_cast<type*>(ptr)) { add_ref();}
        ComPtr(ComPtr const& other) noexcept : ptr_(other.ptr_) { add_ref(); }
        template <typename U> ComPtr(ComPtr<U> const& other) noexcept : ptr_(other.ptr_) { add_ref(); }
        template <typename U> ComPtr(ComPtr<U>&& other) noexcept : ptr_(std::exchange(other.ptr_, {})) { }
        ~ComPtr() noexcept { release_ref(); }

        ComPtr& operator=(ComPtr const& other) noexcept
        {
            copy_ref(other.ptr_);
            return *this;
        }

        ComPtr& operator=(ComPtr&& other) noexcept
        {
            if (this != &other)
            {
                release_ref();
                ptr_ = std::exchange(other.ptr_, {});
            }

            return *this;
        }

        template <typename U>
        ComPtr& operator=(ComPtr<U> const& other) noexcept
        {
            copy_ref(other.ptr_);
            return *this;
        }

        template <typename U>
        ComPtr& operator=(ComPtr<U>&& other) noexcept
        {
            release_ref();
            ptr_ = std::exchange(other.ptr_, {});
            return *this;
        }

        explicit operator bool() const noexcept { return ptr_ != nullptr; }
        auto operator->() const noexcept { return ptr_; }
        T& operator*() const noexcept { return *ptr_; }
        type* get() const noexcept { return ptr_; }
        type** put() noexcept
        {
            ASSERT(ptr_ == nullptr);
            return &ptr_;
        }

        void** put_void() noexcept { return reinterpret_cast<void**>(put()); }
        void attach(type* value) noexcept
        {
            release_ref();
            *put() = value;
        }
        type* detach() noexcept { return std::exchange(ptr_, {}); }
        friend void swap(ComPtr& left, ComPtr& right) noexcept { std::swap(left.ptr_, right.ptr_); }

        template <typename To>
        ComPtr<To> as() const noexcept
        {
            if (!ptr_)
                return nullptr;

            void* result {};
            ASSERT(SUCCEEDED(ptr_->QueryInterface(__uuidof(To), &result)));
            return ComPtr<To>(result, impl::take_ownership_from_abi);
        }

        template <typename To>
        HRESULT try_as(ComPtr<To>& p) const noexcept
        {
            return ptr_->QueryInterface(__uuidof(To), p.put_void());
        }

        HRESULT try_as(REFIID id, void** result) const noexcept
        {
            return ptr_->QueryInterface(id, result);
        }

    private:
        void copy_ref(type* other) noexcept
        {
            if (ptr_ != other)
            {
                release_ref();
                ptr_ = other;
                add_ref();
            }
        }

        void add_ref() const noexcept
        {
            if (!ptr_)
                return;
            
            const_cast<std::remove_const_t<type>*>(ptr_)->AddRef();
        }

        void release_ref() noexcept
        {
            if (!ptr_)
                return;

            unconditional_release_ref();
        }

        NOINLINE void unconditional_release_ref() noexcept
        {
            std::exchange(ptr_, {})->Release();
        }

        template <typename U>
        friend struct ComPtr;

        type* ptr_ {};
    };
}

#undef NOINLINE