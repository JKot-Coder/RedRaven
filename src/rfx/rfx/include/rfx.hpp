#pragma once

#include <stdint.h>

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

        This type is generally compatible with the Windows API `HRESULT` type. In particular, negative values indicate
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

        enum class RfxResult : int32_t
        {
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
            BufferTooSmall = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 1), // Supplied buffer is too small to be able to complete
            Uninitialized = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 2), /* Used to identify a Result that has yet to be initialized.
                                                                        It defaults to failure such that if used incorrectly will fail, as similar in concept to using an uninitialized variable.*/
            Pending = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 3), // Returned from an async method meaning the output is invalid (thus an error), but a result for the request is pending, and will be returned on a subsequent call with the async handle.
            CannotOpen = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 4), // Indicates a file/resource could not be opened
            NotFound = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 5), //!Indicates a file/resource could not be found
            InternalFail = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 6), //! An unhandled internal failure (typically from unhandled exception)
            NotAvailable = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 7), //! Could not complete because some underlying feature (hardware or software) was not available
            TimeOut = RFX_MAKE_ERROR(RFX_FACILITY_CORE, 8) //! Could not complete because the operation times out.
        };

        /* !!!!!!!!!!!!!!!!!!!!! Macros to help checking RfxResult !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
//! Helper macro, that makes it easy to add result checking to calls in functions/methods that themselves return Result.
#define RFX_RETURN_ON_FAIL(x)     \
        {                         \
            RfxResult _res = (x); \
            if (RFX_FAILED(_res)) \
                return _res;      \
        }
//! Helper macro that can be used to test the return value from a call, and will return in a void method/function
#define RFX_RETURN_VOID_ON_FAIL(x) \
        {                          \
            RfxResult _res = (x);  \
            if (RFX_FAILED(_res))  \
                return;            \
        }
//! Helper macro that will return false on failure.
#define RFX_RETURN_FALSE_ON_FAIL(x) \
        {                           \
            RfxResult _res = (x);   \
            if (RFX_FAILED(_res))   \
                return false;       \
        }
//! Helper macro that will return nullptr on failure.
#define RFX_RETURN_NULL_ON_FAIL(x) \
        {                          \
            RfxResult _res = (x);  \
            if (RFX_FAILED(_res))  \
                return nullptr;    \
        }

//! Helper macro that will assert if the return code from a call is failure, also returns the failure.
#define RFX_ASSERT_ON_FAIL(x)     \
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
        {                          \
            RfxResult _res = (x);  \
            if (RFX_FAILED(_res))  \
            {                      \
                ASSERT(false);     \
                return;            \
            }                      \
        }

        enum class CompileTarget : uint32_t
        {
            Dxil,
            Dxil_asm,

            Count
        };

        struct CompilerRequestDescription 
        {
            char* inputFile;
            bool outputPreprocessorResult = false;
        };

        RfxResult Compile(CompilerRequestDescription& compilerRequest, out);
    }
}