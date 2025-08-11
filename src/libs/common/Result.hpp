#pragma once

#define RR_FAILED(status) (int32_t(status) < 0) // Use to test if a result was failure. Never use result != RR_OK to test for failure, as there may be successful codes != RR_OK.
#define RR_SUCCEEDED(status) (int32_t(status) >= 0) // Use to test if a result succeeded. Never use result == RR_OK to test for success, as will detect other successful codes as a failure.

#define RR_GET_RESULT_FACILITY(r) ((int32_t)(((r) >> 16) & 0x7fff)) // Get the facility the result is associated with
#define RR_GET_RESULT_CODE(r) ((int32_t)((r) & 0xffff)) // Get the result code for the facility

#define RR_MAKE_ERROR(fac, code) ((((int32_t)(fac)) << 16) | ((int32_t)(code)) | int32_t(0x80000000))
#define RR_MAKE_SUCCESS(fac, code) ((((int32_t)(fac)) << 16) | ((int32_t)(code)))

/*************************** Facilities ************************************/

//! Facilities compatible with windows COM - only use if known code is compatible
#define RR_FACILITY_WIN_GENERAL 0
#define RR_FACILITY_WIN_INTERFACE 4
#define RR_FACILITY_WIN_API 7
/*! Facilities numbers must be unique across a project to make the resulting result a unique number.
It can be useful to have a consistent short name for a facility, as used in the name prefix */
#define RR_FACILITY_CORE 0x200

/* !!!!!!!!!!!!!!!!!!!!! Macros to help checking RResult !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//! Helper macro, that makes it easy to add result checking to calls in functions/methods that themselves return Result.
#define RR_RETURN_ON_FAIL(x)                  \
    do                                        \
    {                                         \
        const RR::Common::RResult _res = (x); \
        if (RR_FAILED(_res))                  \
            return _res;                      \
    } while (0)

//! Helper macro that can be used to return sucessed result.
#define RR_RETURN_ON_SUCESS(x)                \
    do                                        \
    {                                         \
        const RR::Common::RResult _res = (x); \
        if (RR_SUCCEEDED(_res))               \
            return _res;                      \
    } while (0)

//! Helper macro that can be used to test the return value from a call, and will return in a void method/function
#define RR_RETURN_VOID_ON_FAIL(x) \
    do                            \
    {                             \
        if (RR_FAILED((x)))       \
            return;               \
    } while (0)

//! Helper macro that will return false on failure.
#define RR_RETURN_FALSE_ON_FAIL(x) \
    do                             \
    {                              \
        if (RR_FAILED((x)))        \
            return false;          \
    } while (0)

//! Helper macro that will return nullptr on failure.
#define RR_RETURN_NULL_ON_FAIL(x) \
    do                            \
    {                             \
        if (RR_FAILED((x)))       \
            return nullptr;       \
    } while (0)

//! Helper macro that will return a specific value on failure.
#define RR_RETURN_VALUE_ON_FAIL(x, y) \
    {                                 \
        if (RR_FAILED((x)))           \
            return (y);               \
    }

//! Helper macro that will assert if the return code from a call is failure, also returns the failure.
#define RR_ASSERT_ON_FAIL(x)                  \
    do                                        \
    {                                         \
        const RR::Common::RResult _res = (x); \
        if (RR_FAILED(_res))                  \
        {                                     \
            ASSERT(false);                    \
            return _res;                      \
        }                                     \
    } while (0)

//! Helper macro that will assert if the result from a call is a failure, also returns.
#define RR_ASSERT_VOID_ON_FAIL(x)             \
    do                                        \
    {                                         \
        const RR::Common::RResult _res = (x); \
        if (RR_FAILED(_res))                  \
        {                                     \
            ASSERT(false);                    \
            return;                           \
        }                                     \
    } while (0)

namespace RR::Common
{
    /** A result code for a API operation.

    This Type is generally compatible with the Windows API `HRESULT` Type. In particular, negative values indicate
    failure results, while zero or positive results indicate success.

    In general, APIs always return a zero result on success, unless documented otherwise. Strictly speaking
    a negative value indicates an error, a positive (or 0) value indicates success. This can be tested for with the macros
    RR_SUCCEEDED(x) or RR_FAILED(x).

    It can represent if the call was successful or not. It can also specify in an extensible manner what facility
    produced the result (as the integral 'facility') as well as what caused it (as an integral 'code').
    Under the covers RResult is represented as a int32_t.

    RResult is designed to be compatible with COM HRESULT.

    It's layout in bits is as follows

    Severity | Facility | Code
    ---------|----------|-----
    31       |    30-16 | 15-0

    Severity - 1 fail, 0 is success - as RResult is signed 32 bits, means negative number indicates failure.
    Facility is where the error originated from. Code is the code specific to the facility.
    */

    enum class [[nodiscard]] RResult : int32_t
    {
        Ok = 0, //! RResult::Ok indicates success, and is equivalent to RR_MAKE_SUCCESS(RR_FACILITY_WIN_GENERAL, 0)
        False = 1, //! "RResult::False No error occurred, but indicated false.
        /* ************************ Win COM compatible Results ******************************/
        // https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx
        FileNotFound = RR_MAKE_ERROR(RR_FACILITY_WIN_API, 2), // Indicates a file/resource could not be found (0x80070002)
        AccessDenied = RR_MAKE_ERROR(RR_FACILITY_WIN_API, 5), // General access denied error (0x80070005)
        InvalidHandle = RR_MAKE_ERROR(RR_FACILITY_WIN_API, 6), // Indicates that a handle passed in as parameter to a method is invalid. (0x80070006)
        InvalidArgument = RR_MAKE_ERROR(RR_FACILITY_WIN_API, 0x57), // Indicates that an argument passed in as parameter to a method is invalid. (0x80070057)
        OutOfMemory = RR_MAKE_ERROR(RR_FACILITY_WIN_API, 0xe), // Operation could not complete - ran out of memory (0x8007000E)
        DiskFull = RR_MAKE_ERROR(RR_FACILITY_WIN_API, 0x70), // There is not enough space on the disk.

        NotImplemented = RR_MAKE_ERROR(RR_FACILITY_WIN_GENERAL, 0x4001), // Functionality is not implemented (0x80004001)
        NoInterface = RR_MAKE_ERROR(RR_FACILITY_WIN_GENERAL, 0x4002), // Interface not be found (0x80004002)
        Abort = RR_MAKE_ERROR(RR_FACILITY_WIN_GENERAL, 0x4004), // Operation was aborted (did not correctly complete) (0x80004004)
        Fail = RR_MAKE_ERROR(RR_FACILITY_WIN_GENERAL, 0x4005), // Fail is the generic failure code - meaning a serious error occurred and the call couldn't complete (0x80004005)
        Unexpected = RR_MAKE_ERROR(RR_FACILITY_WIN_GENERAL, 0xFFFF), // Unexpected failure (0x8000FFFF)

        /* *************************** other Results **************************************/
        CannotOpen = RR_MAKE_ERROR(RR_FACILITY_CORE, 4), // Indicates a file/resource could not be opened
        NotFound = RR_MAKE_ERROR(RR_FACILITY_CORE, 5), // Indicates a file/resource could not be found
        AlreadyExist = RR_MAKE_ERROR(RR_FACILITY_CORE, 6), // Indicates a file/resource already exist
        InternalFail = RR_MAKE_ERROR(RR_FACILITY_CORE, 7), //! An unhandled internal failure (typically from unhandled exception)
        NotAvailable = RR_MAKE_ERROR(RR_FACILITY_CORE, 8), //! Could not complete because some underlying feature (hardware or software) was not available
        ArithmeticOverflow = RR_MAKE_ERROR(RR_FACILITY_CORE, 9), //! Arithmetic result exceeded range.
    };

    std::string GetErrorMessage(RResult result);
}