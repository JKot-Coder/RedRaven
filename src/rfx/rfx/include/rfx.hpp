#pragma once

#include <cassert>
#include <stdint.h>
#include <type_traits>

#ifdef _MSC_VER
#define RFX_STDCALL __stdcall
#ifdef RFX_DYNAMIC_EXPORT
#define RFX_DLL_EXPORT __declspec(dllexport)
#else
#define RFX_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define RFX_STDCALL
#define RFX_DLL_EXPORT __attribute__((visibility("default")))
#endif

#define RFX_API RFX_DLL_EXPORT

#ifdef __cplusplus
#define RFX_EXTERN_C extern "C"
#else
#define RFX_EXTERN_C
#endif

namespace RR
{
    namespace Rfx
    {
#define RFX_FAILED(status) (int32_t(status) < 0) // Use to test if a result was failure. Never use result != RFX_OK to test for failure, as there may be successful codes != RFX_OK.
#define RFX_SUCCEEDED(status) (int32_t(status) >= 0) // Use to test if a result succeeded. Never use result == RFX_OK to test for success, as will detect other successful codes as a failure.

#define RFX_GET_RESULT_FACILITY(r) ((int32_t)(((r) >> 16) & 0x7fff)) // Get the facility the result is associated with
#define RFX_GET_RESULT_CODE(r) ((int32_t)((r)&0xffff)) //Get the result code for the facility

#define RFX_MAKE_ERROR(fac, code) ((((int32_t)(fac)) << 16) | ((int32_t)(code)) | int32_t(0x80000000))
#define RFX_MAKE_SUCCESS(fac, code) ((((int32_t)(fac)) << 16) | ((int32_t)(code)))

        /*************************** Facilities ************************************/

        //! Facilities compatible with windows COM - only use if known code is compatible
#define RFX_FACILITY_WIN_GENERAL 0
#define RFX_FACILITY_WIN_INTERFACE 4
#define RFX_FACILITY_WIN_API 7
        /*! Facilities numbers must be unique across a project to make the resulting result a unique number.
        It can be useful to have a consistent short name for a facility, as used in the name prefix */
#define RFX_FACILITY_CORE 0x200

        /** A result code for a Rfx API operation.

        This Type is generally compatible with the Windows API `HRESULT` Type. In particular, negative values indicate
        failure results, while zero or positive results indicate success.

        In general, Rfx APIs always return a zero result on success, unless documented otherwise. Strictly speaking
        a negative value indicates an error, a positive (or 0) value indicates success. This can be tested for with the macros
        RFX_SUCCEEDED(x) or RFX_FAILED(x).

        It can represent if the call was successful or not. It can also specify in an extensible manner what facility
        produced the result (as the integral 'facility') as well as what caused it (as an integral 'code').
        Under the covers RfxResult is represented as a int32_t.

        RfxResult is designed to be compatible with COM HRESULT.

        It's layout in bits is as follows

        Severity | Facility | Code
        ---------|----------|-----
        31       |    30-16 | 15-0

        Severity - 1 fail, 0 is success - as RfxResult is signed 32 bits, means negative number indicates failure.
        Facility is where the error originated from. Code is the code specific to the facility.
        */

        enum class [[nodiscard]] RfxResult : int32_t {
            Ok = 0, //! RFX_OK indicates success, and is equivalent to RFX_MAKE_SUCCESS(RFX_FACILITY_WIN_GENERAL, 0)
            //    False = 1,

            /* ************************ Win COM compatible Results ******************************/
            // https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx
            AccessDenied = RFX_MAKE_ERROR(RFX_FACILITY_WIN_API, 5), // General access denied error (0x80070005)
            InvalidHandle = RFX_MAKE_ERROR(RFX_FACILITY_WIN_API, 6), // Indicates that a handle passed in as parameter to a method is invalid. (0x80070006)
            InvalidArgument = RFX_MAKE_ERROR(RFX_FACILITY_WIN_API, 0x57), // Indicates that an argument passed in as parameter to a method is invalid. (0x80070057)
            OutOfMemory = RFX_MAKE_ERROR(RFX_FACILITY_WIN_API, 0xe), // Operation could not complete - ran out of memory (0x8007000E)

            NotImplemented = RFX_MAKE_ERROR(RFX_FACILITY_WIN_GENERAL, 0x4001), // Functionality is not implemented (0x80004001)
            NoInterface = RFX_MAKE_ERROR(RFX_FACILITY_WIN_GENERAL, 0x4002), // Interface not be found (0x80004002)
            Abort = RFX_MAKE_ERROR(RFX_FACILITY_WIN_GENERAL, 0x4004), // Operation was aborted (did not correctly complete) (0x80004004)
            Fail = RFX_MAKE_ERROR(RFX_FACILITY_WIN_GENERAL, 0x4005), // Fail is the generic failure code - meaning a serious error occurred and the call couldn't complete (0x80004005)
            Unexpected = RFX_MAKE_ERROR(RFX_FACILITY_WIN_GENERAL, 0xFFFF), // Unexpected failure (0x8000FFFF)

            /* *************************** other Results **************************************/
            CannotOpen = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 4), // Indicates a file/resource could not be opened
            NotFound = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 5), //!Indicates a file/resource could not be found
            InternalFail = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 6), //! An unhandled internal failure (typically from unhandled exception)
            NotAvailable = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 7), //! Could not complete because some underlying feature (hardware or software) was not available
        };

        /* !!!!!!!!!!!!!!!!!!!!! Macros to help checking RfxResult !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//! Helper macro, that makes it easy to add result checking to calls in functions/methods that themselves return Result.
#define RFX_RETURN_ON_FAIL(x) \
    {                         \
        RfxResult _res = (x); \
        if (RFX_FAILED(_res)) \
            return _res;      \
    }
//! Helper macro that can be used to test the return value from a call, and will return in a void method/function
#define RFX_RETURN_VOID_ON_FAIL(x) \
    {                              \
        RfxResult _res = (x);      \
        if (RFX_FAILED(_res))      \
            return;                \
    }
//! Helper macro that will return false on failure.
#define RFX_RETURN_FALSE_ON_FAIL(x) \
    {                               \
        RfxResult _res = (x);       \
        if (RFX_FAILED(_res))       \
            return false;           \
    }
//! Helper macro that will return nullptr on failure.
#define RFX_RETURN_NULL_ON_FAIL(x) \
    {                              \
        RfxResult _res = (x);      \
        if (RFX_FAILED(_res))      \
            return nullptr;        \
    }

//! Helper macro that will assert if the return code from a call is failure, also returns the failure.
#define RFX_ASSERT_ON_FAIL(x) \
    {                         \
        RfxResult _res = (x); \
        if (RFX_FAILED(_res)) \
        {                     \
            ASSERT(false);    \
            return _res;      \
        }                     \
    }
//! Helper macro that will assert if the result from a call is a failure, also returns.
#define RFX_ASSERT_VOID_ON_FAIL(x) \
    {                              \
        RfxResult _res = (x);      \
        if (RFX_FAILED(_res))      \
        {                          \
            ASSERT(false);         \
            return;                \
        }                          \
    }

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

            __declspec(noinline) void unconditional_release_ref() noexcept
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

        enum class CompileTarget : uint32_t
        {
            Dxil,
            Dxil_asm,

            Count
        };

        struct CompileRequestDescription
        {
            enum class OutputStage : uint32_t
            {
                Lexer,
                Preprocessor,
                Compiler,
            };

            OutputStage outputStage;

            const char* inputFile;
            const char** defines;
            size_t defineCount;
            
            struct CompilerOptions
            {
                bool assemblyOutput;
                bool objectOutput;
            } compilerOptions;          
        };

        class IRfxUnknown
        {
        public:
            virtual uint32_t addRef() = 0;
            virtual uint32_t release() = 0;
        };

        class IBlob : public IRfxUnknown
        {
        public:
            virtual const void* GetBufferPointer() const = 0;
            virtual size_t GetBufferSize() const = 0;
        };

        enum class CompileOutputType
        {
            Diagnostic,
            Assembly,
            Tokens, // The code after lexer stage
            Source, // The code after preprossing stage
            Object
        };

        class ICompileResult : public IRfxUnknown
        {
        public:
            virtual RfxResult GetOutput(size_t index, CompileOutputType& outputType, IBlob** output) = 0;
            virtual size_t GetOutputsCount() = 0;
        };

        class ICompiler : public IRfxUnknown
        {
        public:
            virtual RfxResult Compile(const CompileRequestDescription& compilerRequest, ICompileResult** result) = 0;
        };

        RFX_EXTERN_C RFX_API RfxResult RFX_STDCALL GetComplierInstance(ICompiler** compiler);
        RFX_EXTERN_C RFX_API RfxResult RFX_STDCALL GetErrorMessage(RfxResult result, IBlob** message);
    }
}