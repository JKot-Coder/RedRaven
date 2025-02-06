#pragma once

#include <EASTL/tuple.h>
#include <EASTL/type_traits.h>

namespace RR::Ecs
{
    /**
     * @brief List of function argument types with compile-time access
     * @tparam Args Variadic template parameter pack of argument types
     */
    template <typename... Args>
    struct ArgumentList
    {
        static constexpr size_t Count = sizeof...(Args);

        template <size_t Index>
        using Get = eastl::tuple_element_t<Index, eastl::tuple<Args...>>;
    };

    /**
     * @brief Base traits for function types
     * @tparam RetType Function return type
     * @tparam Args Function argument types
     */
    template <typename RetType, typename... Args>
    struct FunctionTraitsBase
    {
        using ReturnType = RetType;
        using Arguments = ArgumentList<Args...>;

        static constexpr bool IsCallable = true;
        static constexpr size_t ArgsCount = sizeof...(Args);

        template <size_t Index>
        using Argument = typename Arguments::template Get<Index>;
    };

    namespace Detail
    {
        // Primary template for function traits implementation
        template <typename T>
        struct FunctionTraitsImpl
        {
            static constexpr bool IsCallable = false;
        };

        // Free function specialization
        template <typename ReturnType, typename... Args>
        struct FunctionTraitsImpl<ReturnType(Args...)>
            : FunctionTraitsBase<ReturnType, Args...>
        {
        };

        // Function pointer specialization
        template <typename ReturnType, typename... Args>
        struct FunctionTraitsImpl<ReturnType (*)(Args...)>
            : FunctionTraitsBase<ReturnType, Args...>
        {
        };

// Member function specializations with CV and ref qualifiers
#define MAKE_MEMBER_FUNCTION_TRAITS(CV_REF_QUALIFIER)                              \
    template <typename ClassType, typename ReturnType, typename... Args>           \
    struct FunctionTraitsImpl<ReturnType (ClassType::*)(Args...) CV_REF_QUALIFIER> \
        : FunctionTraitsBase<ReturnType, Args...>                                  \
    {                                                                              \
    }

        MAKE_MEMBER_FUNCTION_TRAITS();
        MAKE_MEMBER_FUNCTION_TRAITS(const);
        MAKE_MEMBER_FUNCTION_TRAITS(volatile);
        MAKE_MEMBER_FUNCTION_TRAITS(const volatile);
        MAKE_MEMBER_FUNCTION_TRAITS(&);
        MAKE_MEMBER_FUNCTION_TRAITS(const&);
        MAKE_MEMBER_FUNCTION_TRAITS(volatile&);
        MAKE_MEMBER_FUNCTION_TRAITS(const volatile&);
        MAKE_MEMBER_FUNCTION_TRAITS(&&);
        MAKE_MEMBER_FUNCTION_TRAITS(const&&);
        MAKE_MEMBER_FUNCTION_TRAITS(volatile&&);
        MAKE_MEMBER_FUNCTION_TRAITS(const volatile&&);

#undef MAKE_MEMBER_FUNCTION_TRAITS

        // Callable object traits (lambdas, functors)
        template <typename T, typename = void>
        struct FunctionTraitsNoCV : FunctionTraitsImpl<T>
        {
        };

        template <typename T>
        struct FunctionTraitsNoCV<T, decltype((void)&T::operator())>
            : FunctionTraitsImpl<decltype(&T::operator())>
        {
        };
    }

    /**
     * @brief Main traits interface for function types
     * @tparam T Function type to inspect
     */
    template <typename T>
    struct FunctionTraits : Detail::FunctionTraitsNoCV<eastl::decay_t<T>>
    {
    };

    /**
     * @brief Helper type traits and constants for common operations
     */
    template <typename T>
    inline constexpr bool IsCallableV = FunctionTraits<T>::IsCallable;

    template <typename T>
    using GetReturnType = typename FunctionTraits<T>::ReturnType;

    template <typename T>
    using GetArgumentList = typename FunctionTraits<T>::Arguments;

    template <typename T, size_t Index>
    using GetArgument = typename FunctionTraits<T>::template Argument<Index>;

    template <typename T>
    inline constexpr size_t GetArgumentsCount = FunctionTraits<T>::ArgsCount;
}