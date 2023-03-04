#pragma once

#ifdef _MSC_VER
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

namespace RR::Common
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

    template <typename T>
    class ComPtr
    {
    public:
        using Type = abi_t<T>;

        ~ComPtr() noexcept { releaseRef(); }

        ComPtr(std::nullptr_t = nullptr) noexcept { }
        ComPtr(void* ptr) noexcept : ptr_(static_cast<Type*>(ptr)) { addRef(); }
        ComPtr(ComPtr const& other) noexcept : ptr_(other.ptr_) { addRef(); }

        template <typename U>
        ComPtr(ComPtr<U> const& other) noexcept : ptr_(other.ptr_) { addRef(); }

        template <typename U>
        ComPtr(ComPtr<U>&& other) noexcept : ptr_(std::exchange(other.ptr_, {})) { }

        ComPtr& operator=(ComPtr const& other) noexcept
        {
            copyRef(other.ptr_);
            return *this;
        }

        ComPtr& operator=(ComPtr&& other) noexcept
        {
            if (this != &other)
            {
                releaseRef();
                ptr_ = std::exchange(other.ptr_, {});
            }

            return *this;
        }

        template <typename U>
        ComPtr& operator=(ComPtr<U> const& other) noexcept
        {
            copyRef(other.ptr_);
            return *this;
        }

        template <typename U>
        ComPtr& operator=(ComPtr<U>&& other) noexcept
        {
            releaseRef();
            ptr_ = std::exchange(other.ptr_, {});
            return *this;
        }

        explicit operator bool() const noexcept { return ptr_ != nullptr; }
        auto operator->() const noexcept { return ptr_; }
        T& operator*() const noexcept { return *ptr_; }

        Type* get() const noexcept { return ptr_; }
        Type** put() noexcept
        {
            assert(ptr_ == nullptr);
            return &ptr_;
        }

        void** put_void() noexcept { return reinterpret_cast<void**>(put()); }

        void attach(Type* value) noexcept
        {
            releaseRef();
            *put() = value;
        }

        Type* detach() noexcept { return std::exchange(ptr_, {}); }

        friend void swap(ComPtr& left, ComPtr& right) noexcept
        {
            std::swap(left.ptr_, right.ptr_);
        }

        void copyTo(Type** other) const noexcept
        {
            addRef();
            *other = ptr_;
        }

    private:
        void copyRef(Type* other) noexcept
        {
            if (ptr_ != other)
            {
                releaseRef();
                ptr_ = other;
                addRef();
            }
        }

        void addRef() const noexcept
        {
            if (ptr_)
                const_cast<std::remove_const_t<Type>*>(ptr_)->addRef();
        }

        void releaseRef() noexcept
        {
            if (ptr_)
                unconditional_release_ref();
        }

        NOINLINE void unconditional_release_ref() noexcept
        {
            std::exchange(ptr_, {})->release();
        }

        template <typename U>
        friend struct com_ptr;

        Type* ptr_ {};
    };

    template <typename T>
    bool operator==(ComPtr<T> const& left, ComPtr<T> const& right) noexcept
    {
        return left.get() == right.get();
    }

    template <typename T>
    bool operator==(ComPtr<T> const& left, std::nullptr_t) noexcept
    {
        return left.get() == nullptr;
    }

    template <typename T>
    bool operator==(std::nullptr_t, ComPtr<T> const& right) noexcept
    {
        return right.get();
    }

    template <typename T>
    bool operator!=(ComPtr<T> const& left, ComPtr<T> const& right) noexcept
    {
        return !(left == right);
    }

    template <typename T>
    bool operator!=(ComPtr<T> const& left, std::nullptr_t) noexcept
    {
        return !(left == nullptr);
    }

    template <typename T>
    bool operator!=(std::nullptr_t, ComPtr<T> const& right) noexcept
    {
        return !(nullptr == right);
    }

    template <typename T>
    bool operator<(ComPtr<T> const& left, ComPtr<T> const& right) noexcept
    {
        return left.get() < right.get();
    }

    template <typename T>
    bool operator>(ComPtr<T> const& left, ComPtr<T> const& right) noexcept
    {
        return right < left;
    }

    template <typename T>
    bool operator<=(ComPtr<T> const& left, ComPtr<T> const& right) noexcept
    {
        return !(right < left);
    }

    template <typename T>
    bool operator>=(ComPtr<T> const& left, ComPtr<T> const& right) noexcept
    {
        return !(left < right);
    }
}

#undef NOINLINE