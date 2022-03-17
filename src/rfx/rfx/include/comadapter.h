// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

// These #defines prevent the idl-generated headers from trying to include
// Windows.h from the SDK rather than this one.
#define RPC_NO_WINDOWS_H
#define COM_NO_WINDOWS_H

#include <stdint.h>

// Note: using fixed-width here to match Windows widths
// Specifically this is different for 'long' vs 'LONG'
typedef uint8_t UINT8;
typedef int8_t INT8;
typedef uint16_t UINT16;
typedef int16_t INT16;
typedef uint32_t UINT32, UINT, ULONG, DWORD, BOOL;
typedef int32_t INT32, INT, LONG;

// Misc defines
#define interface struct
#define MIDL_INTERFACE(x) interface
#define DECLSPEC_UUID(x)
#define DECLSPEC_NOVTABLE

// SAL annotations
#define _In_
#define _In_z_
#define _In_opt_
#define _In_opt_z_
#define _In_reads_(x)
#define _In_reads_opt_(x)
#define _In_reads_bytes_(x)
#define _In_reads_bytes_opt_(x)
#define _In_range_(x, y)
#define _In_bytecount_(x)
#define _Out_
#define _Out_opt_
#define _Outptr_
#define _Outptr_opt_result_z_
#define _Outptr_opt_result_bytebuffer_(x)
#define _COM_Outptr_
#define _COM_Outptr_result_maybenull_
#define _COM_Outptr_opt_
#define _COM_Outptr_opt_result_maybenull_
#define _Out_writes_(x)
#define _Out_writes_z_(x)
#define _Out_writes_opt_(x)
#define _Out_writes_all_(x)
#define _Out_writes_all_opt_(x)
#define _Out_writes_to_opt_(x, y)
#define _Out_writes_bytes_(x)
#define _Out_writes_bytes_all_(x)
#define _Out_writes_bytes_all_opt_(x)
#define _Out_writes_bytes_opt_(x)
#define _Inout_
#define _Inout_opt_
#define _Inout_updates_(x)
#define _Inout_updates_bytes_(x)
#define _Field_size_(x)
#define _Field_size_opt_(x)
#define _Field_size_bytes_(x)
#define _Field_size_full_(x)
#define _Field_size_bytes_full_(x)
#define _Field_size_bytes_full_opt_(x)
#define _Field_size_bytes_part_(x, y)
#define _Field_range_(x, y)
#define _Field_z_
#define _Check_return_
#define _IRQL_requires_(x)
#define _IRQL_requires_min_(x)
#define _IRQL_requires_max_(x)
#define _At_(x, y)
#define _Always_(x)
#define _Return_type_success_(x)
#define _Translates_Win32_to_HRESULT_(x)
#define _Maybenull_
#define _Outptr_result_maybenull_
#define _Outptr_result_nullonfailure_
#define _Analysis_assume_(x)
#define _Success_(x)
#define _In_count_(x)
#define _In_opt_count_(x)
#define _Use_decl_annotations_

// Calling conventions
#define STDMETHODCALLTYPE

typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[ 8 ];
} GUID;

#ifdef __cplusplus
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern "C" const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern "C" const GUID name
#endif

template <typename T> GUID uuidof() = delete;
template <typename T> GUID uuidof(T*) { return uuidof<T>(); }
template <typename T> GUID uuidof(T**) { return uuidof<T>(); }
template <typename T> GUID uuidof(T&) { return uuidof<T>(); }
#define __uuidof(x) uuidof(x)
#else
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) extern const GUID name
#endif
#endif

typedef GUID IID;
typedef GUID UUID;
typedef GUID CLSID;
#ifdef __cplusplus
#define REFGUID const GUID &
#define REFIID const IID &
#define REFCLSID const IID &

__inline int InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return (
        ((uint32_t *)&rguid1)[0] == ((uint32_t *)&rguid2)[0] &&
        ((uint32_t *)&rguid1)[1] == ((uint32_t *)&rguid2)[1] &&
        ((uint32_t *)&rguid1)[2] == ((uint32_t *)&rguid2)[2] &&
        ((uint32_t *)&rguid1)[3] == ((uint32_t *)&rguid2)[3]);
}

inline bool operator==(REFGUID guidOne, REFGUID guidOther)
{
    return !!InlineIsEqualGUID(guidOne, guidOther);
}

inline bool operator!=(REFGUID guidOne, REFGUID guidOther)
{
    return !(guidOne == guidOther);
}

#else
#define REFGUID const GUID *
#define REFIID const IID *
#define REFCLSID const IID *
#endif

// Error codes
typedef LONG HRESULT;
#define SUCCEEDED(hr)  (((HRESULT)(hr)) >= 0)
#define FAILED(hr)     (((HRESULT)(hr)) < 0)
#define S_OK           ((HRESULT)0L)
#define S_FALSE        ((HRESULT)1L)
#define E_NOTIMPL      ((HRESULT)0x80004001L)
#define E_OUTOFMEMORY  ((HRESULT)0x8007000EL)
#define E_INVALIDARG   ((HRESULT)0x80070057L)
#define E_NOINTERFACE  ((HRESULT)0x80004002L)
#define E_POINTER      ((HRESULT)0x80004003L)
#define E_HANDLE       ((HRESULT)0x80070006L)
#define E_ABORT        ((HRESULT)0x80004004L)
#define E_FAIL         ((HRESULT)0x80004005L)
#define E_ACCESSDENIED ((HRESULT)0x80070005L)
#define E_UNEXPECTED   ((HRESULT)0x8000FFFFL)

#ifdef __cplusplus
// IUnknown

interface DECLSPEC_UUID("00000000-0000-0000-C000-000000000046") DECLSPEC_NOVTABLE IUnknown
{
   virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) = 0;
   virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
   virtual ULONG STDMETHODCALLTYPE Release() = 0;

   template <class Q> HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp) {
       return QueryInterface(uuidof<Q>(), (void **)pp);
   }
};

template <> constexpr GUID uuidof<IUnknown>()
{
    return { 0x00000000, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
}

extern "C++"
{
    template<typename T> void** IID_PPV_ARGS_Helper(T** pp)
    {
        static_cast<IUnknown*>(*pp);
        return reinterpret_cast<void**>(pp);
    }
}

#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType)
#endif