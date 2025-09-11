#pragma once

#include <limits>
#include <stdint.h>
#include <type_traits>
#include "common/hashing/HashType.hpp"

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else // defined(_MSC_VER)
#define FORCE_INLINE __attribute__((always_inline)) inline
#endif // !defined(_MSC_VER)

namespace RR::Common::Hashing::Wyhash
{
    namespace details
    {
        template <size_t HashBits>
        using HashType = typename HashType_t<HashBits>::type;

        template <class T>
        constexpr FORCE_INLINE T ROTL(const T v, uint32_t R)
        {
#if __cplusplus >= 202002L
            static_assert(false, "Replace it with std::rotl");
#endif
            static_assert(std::is_unsigned<T>::value, "Can only rotate unsigned integral types");
            return (v << (R % std::numeric_limits<T>::digits)) |
                   (v >> (std::numeric_limits<T>::digits - (R % std::numeric_limits<T>::digits)));
        }

        constexpr FORCE_INLINE uint64_t castChar(char c)
        {
            return static_cast<uint64_t>(static_cast<uint8_t>(c));
        };

        template <uint32_t BitNum, bool Constexpr>
        constexpr FORCE_INLINE uint64_t readBlock(const char* p)
        {

#if __cplusplus >= 202002L
            static_assert(false, "Replace Constexpr with is_constant_evaluated");
#endif

            if (Constexpr)
            {
                switch (BitNum)
                {
                    case 8: return static_cast<uint8_t>(p[0]);
                    case 16: return castChar(p[0]) << 0 |
                                    castChar(p[1]) << 8;
                    case 32: return castChar(p[0]) << 0 |
                                    castChar(p[1]) << 8 |
                                    castChar(p[2]) << 16 |
                                    castChar(p[3]) << 24;
                    case 64: return castChar(p[0]) << 0 |
                                    castChar(p[1]) << 8 |
                                    castChar(p[2]) << 16 |
                                    castChar(p[3]) << 24 |
                                    castChar(p[4]) << 32 |
                                    castChar(p[5]) << 40 |
                                    castChar(p[6]) << 48 |
                                    castChar(p[7]) << 56;
                }
                return 0;
                static_assert(BitNum == 8 || BitNum == 16 || BitNum == 32 || BitNum == 64, "Wrong number of bits");
            }
            else
            {
                HashType<BitNum> v;
                memcpy(&v, p, sizeof(HashType<BitNum>));
                return v;
            }
        }

        template <bool Constexpr>
        constexpr FORCE_INLINE uint64_t readBlock64(const char* p) { return (readBlock<32, Constexpr>(p) << 32) | readBlock<32, Constexpr>(p + 4); }

        // -------------------------------------------------------------------------------------------------------------------------------
        constexpr FORCE_INLINE uint64_t watermum(const uint64_t A, const uint64_t B)
        {
            uint64_t r = A * B;
            return r - (r >> 32);
        }

        /*
            Waterhash takes (optimally) 32-bit inputs and produces a 32-bit hash as its result.
            It is an edited version of wyhash that uses at most 64-bit math instead of 128-bit.
            It is meant to use very similar code to Wheathash, which produces a 64-bit hash.
            Original Author: Wang Yi <godspeed_china@yeah.net>
            Waterhash Variant Author: Tommy Ettinger <tommy.ettinger@gmail.com>
        */
        template <bool CE>
        constexpr FORCE_INLINE uint32_t waterhash(const char* key, uint32_t len, uint64_t seed)
        {
            constexpr uint64_t _waterp0 = 0xa0761d65ull, _waterp1 = 0xe7037ed1ull, _waterp2 = 0x8ebc6af1ull;
            constexpr uint64_t _waterp3 = 0x589965cdull, _waterp4 = 0x1d8e4e27ull, _waterp5 = 0xeb44accbull;

            const char* p = key;
            for (uint32_t i = 0; i + 16 <= len; i += 16, p += 16)
            {
                seed = watermum(
                    watermum(readBlock<32, CE>(p) ^ _waterp1, readBlock<32, CE>(p + 4) ^ _waterp2) + seed,
                    watermum(readBlock<32, CE>(p + 8) ^ _waterp3, readBlock<32, CE>(p + 12) ^ _waterp4));
            }
            seed += _waterp5;
            switch (len & 15)
            {
                case 1: seed = watermum(_waterp2 ^ seed, readBlock<8, CE>(p) ^ _waterp1); break;
                case 2: seed = watermum(_waterp3 ^ seed, readBlock<16, CE>(p) ^ _waterp4); break;
                case 3: seed = watermum(readBlock<16, CE>(p) ^ seed, readBlock<8, CE>(p + 2) ^ _waterp2); break;
                case 4: seed = watermum(readBlock<16, CE>(p) ^ seed, readBlock<16, CE>(p + 2) ^ _waterp3); break;
                case 5: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<8, CE>(p + 4) ^ _waterp1); break;
                case 6: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<16, CE>(p + 4) ^ _waterp1); break;
                case 7: seed = watermum(readBlock<32, CE>(p) ^ seed, (readBlock<16, CE>(p + 4) << 8 | readBlock<8, CE>(p + 6)) ^ _waterp1); break;
                case 8: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp0); break;
                case 9: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed ^ _waterp4, readBlock<8, CE>(p + 8) ^ _waterp3); break;
                case 10: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed, readBlock<16, CE>(p + 8) ^ _waterp3); break;
                case 11: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed, ((readBlock<16, CE>(p + 8) << 8) | readBlock<8, CE>(p + 10)) ^ _waterp3); break;
                case 12: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed ^ readBlock<32, CE>(p + 8), _waterp4); break;
                case 13: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed ^ readBlock<32, CE>(p + 8), (readBlock<8, CE>(p + 12)) ^ _waterp4); break;
                case 14: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed ^ readBlock<32, CE>(p + 8), (readBlock<16, CE>(p + 12)) ^ _waterp4); break;
                case 15: seed = watermum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _waterp2) ^ watermum(seed ^ readBlock<32, CE>(p + 8), (readBlock<16, CE>(p + 12) << 8 | readBlock<8, CE>(p + 14)) ^ _waterp4); break;
            }
            seed = (seed ^ seed << 16) * (len ^ _waterp0);
            return (uint32_t)(seed - (seed >> 32));
        }
        // -------------------------------------------------------------------------------------------------------------------------------
        constexpr FORCE_INLINE uint64_t wheatmum(uint64_t A, uint64_t B)
        {
            uint64_t r = A * B;
            return r - (r >> 32);
        }

        /*
            Wheathash takes (optimally) 32-bit inputs and produces a 64-bit hash as its result.
            It is a slightly-edited version of Waterhash, which is an edited version of wyhash.
            It is meant to use very similar code to Waterhash, which produces a 32-bit hash.
            Original Author: Wang Yi <godspeed_china@yeah.net>
            Wheathash Variant Author: Tommy Ettinger <tommy.ettinger@gmail.com>
        */
        template <bool CE>
        constexpr FORCE_INLINE uint64_t wheathash(const char* key, uint32_t len, uint64_t seed)
        {
            constexpr uint64_t _wheatp0 = 0xa0761d6478bd642full, _wheatp1 = 0xe7037ed1a0b428dbull, _wheatp2 = 0x8ebc6af09c88c6e3ull;
            constexpr uint64_t _wheatp3 = 0x589965cc75374cc3ull, _wheatp4 = 0x1d8e4e27c47d124full, _wheatp5 = 0xeb44accab455d165ull;

            const char* p = key;
            for (uint32_t i = 0; i + 16 <= len; i += 16, p += 16)
            {
                seed = wheatmum(
                    wheatmum(readBlock<32, CE>(p) ^ _wheatp1, readBlock<32, CE>(p + 4) ^ _wheatp2) + seed,
                    wheatmum(readBlock<32, CE>(p + 8) ^ _wheatp3, readBlock<32, CE>(p + 12) ^ _wheatp4));
            }
            seed += _wheatp5;
            switch (len & 15)
            {
                case 1: seed = wheatmum(_wheatp2 ^ seed, readBlock<8, CE>(p) ^ _wheatp1); break;
                case 2: seed = wheatmum(_wheatp3 ^ seed, readBlock<16, CE>(p) ^ _wheatp4); break;
                case 3: seed = wheatmum(readBlock<16, CE>(p) ^ seed, readBlock<8, CE>(p + 2) ^ _wheatp2); break;
                case 4: seed = wheatmum(readBlock<16, CE>(p) ^ seed, readBlock<16, CE>(p + 2) ^ _wheatp3); break;
                case 5: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<8, CE>(p + 4) ^ _wheatp1); break;
                case 6: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<16, CE>(p + 4) ^ _wheatp1); break;
                case 7: seed = wheatmum(readBlock<32, CE>(p) ^ seed, (readBlock<16, CE>(p + 4) << 8 | readBlock<8, CE>(p + 6)) ^ _wheatp1); break;
                case 8: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp0); break;
                case 9: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed ^ _wheatp4, readBlock<8, CE>(p + 8) ^ _wheatp3); break;
                case 10: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed, readBlock<16, CE>(p + 8) ^ _wheatp3); break;
                case 11: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed, ((readBlock<16, CE>(p + 8) << 8) | readBlock<8, CE>(p + 10)) ^ _wheatp3); break;
                case 12: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed ^ readBlock<32, CE>(p + 8), _wheatp4); break;
                case 13: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed ^ readBlock<32, CE>(p + 8), (readBlock<8, CE>(p + 12)) ^ _wheatp4); break;
                case 14: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed ^ readBlock<32, CE>(p + 8), (readBlock<16, CE>(p + 12)) ^ _wheatp4); break;
                case 15: seed = wheatmum(readBlock<32, CE>(p) ^ seed, readBlock<32, CE>(p + 4) ^ _wheatp2) ^ wheatmum(seed ^ readBlock<32, CE>(p + 8), (readBlock<16, CE>(p + 12) << 8 | readBlock<8, CE>(p + 14)) ^ _wheatp4); break;
            }
            seed = (seed ^ seed << 16) * (len ^ _wheatp0);
            return seed - (seed >> 31) + (seed << 33);
        }

        // -------------------------------------------------------------------------------------------------------------------------------
        // has an uneven rotation structure that gives resistance to when one parameter is 0, unlike wyhash' _wymum().
        constexpr FORCE_INLINE uint64_t wootmum(const uint64_t A, const uint64_t B)
        {
            uint64_t r = (A ^ ROTL<uint64_t>(B, 39)) * (B ^ ROTL<uint64_t>(A, 39));
            return r - (r >> 32);
        }

        /*
            Woothash takes (optimally) 64-bit inputs and produces a 64-bit hash as its result.
            It is an edited version of wyhash that uses at most 64-bit math instead of 128-bit.
            Version 2 is a little slower than version 1, but the reverse is true on the JVM.
            Original Author: Wang Yi <godspeed_china@yeah.net>
            Woothash Variant Author: Tommy Ettinger <tommy.ettinger@gmail.com>
        */
        template <bool CE>
        constexpr FORCE_INLINE uint64_t woothash(const char* key, uint64_t len, uint64_t seed)
        {
            constexpr uint64_t _wootp0 = 0xa0761d6478bd642full, _wootp1 = 0xe7037ed1a0b428dbull, _wootp2 = 0x8ebc6af09c88c6e3ull;
            constexpr uint64_t _wootp3 = 0x589965cc75374cc3ull, _wootp4 = 0x1d8e4e27c47d124full, _wootp5 = 0xeb44accab455d165ull;

            const char* p = key;
            uint64_t a = seed ^ _wootp4, b = ROTL<uint64_t>(seed, 17) ^ _wootp3, c = ROTL<uint64_t>(seed, 31) ^ _wootp2, d = ROTL<uint64_t>(seed, 47) ^ _wootp1;
            for (uint64_t i = 0; i + 32 <= len; i += 32, p += 32)
            {
                a = (readBlock<64, CE>(p) ^ a) * _wootp1;
                a = ROTL<uint64_t>(a, 22);
                a *= _wootp3;
                b = (readBlock<64, CE>(p + 8) ^ b) * _wootp2;
                b = ROTL<uint64_t>(b, 25);
                b *= _wootp4;
                c = (readBlock<64, CE>(p + 16) ^ c) * _wootp3;
                c = ROTL<uint64_t>(c, 28);
                c *= _wootp5;
                d = (readBlock<64, CE>(p + 24) ^ d) * _wootp4;
                d = ROTL<uint64_t>(d, 31);
                d *= _wootp1;
                seed += a + b + c + d;
            }
            seed += _wootp5;
            switch (len & 31)
            {
                case 1: seed = wootmum(seed, readBlock<8, CE>(p) ^ _wootp1); break;
                case 2: seed = wootmum(seed, readBlock<16, CE>(p) ^ _wootp1); break;
                case 3: seed = wootmum(seed, ((readBlock<16, CE>(p) << 8) | readBlock<8, CE>(p + 2)) ^ _wootp1); break;
                case 4: seed = wootmum(seed, readBlock<32, CE>(p) ^ _wootp1); break;
                case 5: seed = wootmum(seed, ((readBlock<32, CE>(p) << 8) | readBlock<8, CE>(p + 4)) ^ _wootp1); break;
                case 6: seed = wootmum(seed, ((readBlock<32, CE>(p) << 16) | readBlock<16, CE>(p + 4)) ^ _wootp1); break;
                case 7: seed = wootmum(seed, ((readBlock<32, CE>(p) << 24) | (readBlock<16, CE>(p + 4) << 8) | readBlock<8, CE>(p + 6)) ^ _wootp1); break;
                case 8: seed = wootmum(seed, readBlock64<CE>(p) ^ _wootp1); break;
                case 9: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<8, CE>(p + 8) ^ _wootp2); break;
                case 10: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<16, CE>(p + 8) ^ _wootp2); break;
                case 11: seed = wootmum(readBlock64<CE>(p) + seed, ((readBlock<16, CE>(p + 8) << 8) | readBlock<8, CE>(p + 8 + 2)) ^ _wootp2); break;
                case 12: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<32, CE>(p + 8) ^ _wootp2); break;
                case 13: seed = wootmum(readBlock64<CE>(p) + seed, ((readBlock<32, CE>(p + 8) << 8) | readBlock<8, CE>(p + 8 + 4)) ^ _wootp2); break;
                case 14: seed = wootmum(readBlock64<CE>(p) + seed, ((readBlock<32, CE>(p + 8) << 16) | readBlock<16, CE>(p + 8 + 4)) ^ _wootp2); break;
                case 15: seed = wootmum(readBlock64<CE>(p) + seed, ((readBlock<32, CE>(p + 8) << 24) | (readBlock<16, CE>(p + 8 + 4) << 8) | readBlock<8, CE>(p + 8 + 6)) ^ _wootp2); break;
                case 16: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2); break;
                case 17: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<32, CE>(p + 8) + _wootp2) + wootmum(readBlock<32, CE>(p + 12) ^ seed, readBlock<8, CE>(p + 16) ^ _wootp3); break;
                case 18: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<32, CE>(p + 8) + _wootp2) + wootmum(readBlock<32, CE>(p + 12) ^ seed, readBlock<16, CE>(p + 16) ^ _wootp3); break;
                case 19: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<32, CE>(p + 8) + _wootp2) + wootmum(readBlock<32, CE>(p + 12) ^ seed, ((readBlock<16, CE>(p + 16) << 8) | readBlock<8, CE>(p + 16 + 2)) ^ _wootp3); break;
                case 20: seed = wootmum(readBlock64<CE>(p) + seed, readBlock<32, CE>(p + 8) + _wootp2) + wootmum(readBlock<32, CE>(p + 12) ^ seed, readBlock<32, CE>(p + 16) ^ _wootp3); break;
                case 21: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock<16, CE>(p + 16) ^ seed, ((readBlock<16, CE>(p + 18) << 8) | readBlock<8, CE>(p + 16 + 4)) ^ _wootp3); break;
                case 22: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock<16, CE>(p + 16) ^ seed, (readBlock<32, CE>(p + 18) << 16) ^ _wootp3); break;
                case 23: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(_wootp4 + seed, ((readBlock<32, CE>(p + 16) << 24) | (readBlock<16, CE>(p + 16 + 4) << 8) | readBlock<8, CE>(p + 16 + 6)) ^ _wootp3); break;
                case 24: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) + seed, seed ^ _wootp3); break;
                case 25: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, readBlock<8, CE>(p + 24) ^ _wootp4); break;
                case 26: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, readBlock<16, CE>(p + 24) ^ _wootp4); break;
                case 27: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, ((readBlock<16, CE>(p + 24) << 8) | readBlock<8, CE>(p + 24 + 2)) ^ _wootp4); break;
                case 28: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, readBlock<32, CE>(p + 24) ^ _wootp4); break;
                case 29: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, ((readBlock<32, CE>(p + 24) << 8) | readBlock<8, CE>(p + 24 + 4)) ^ _wootp4); break;
                case 30: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, ((readBlock<32, CE>(p + 24) << 16) | readBlock<16, CE>(p + 24 + 4)) ^ _wootp4); break;
                case 31: seed = wootmum(readBlock64<CE>(p) + seed, readBlock64<CE>(p + 8) + _wootp2) + wootmum(readBlock64<CE>(p + 16) ^ seed, ((readBlock<32, CE>(p + 24) << 24) | (readBlock<16, CE>(p + 24 + 4) << 8) | readBlock<8, CE>(p + 24 + 6)) ^ _wootp4); break;
            }
            seed = (seed ^ seed << 16) * (len ^ _wootp0 ^ seed >> 32);
            return seed - (seed >> 31) + (seed << 33);
        }
        // -------------------------------------------------------------------------------------------------------------------------------

        static_assert(waterhash<true>("waterhash", 8, 0x59B8541C) == 0x29dcbc19, "SanityCheck");
        static_assert(wheathash<true>("wheathash", 8, 0x7625AEEC) == 0x928f593f70055e88, "SanityCheck");
        static_assert(woothash<true>("woothash", 8, 0x23968DAB) == 0xacf137bfd07e6073, "SanityCheck");
    }

    template <uint32_t BitNum> // By default water for 32 bits and woot for 64 bit
    constexpr FORCE_INLINE details::HashType<BitNum> Hash(const void* key, size_t len);

    template <uint32_t BitNum> // By default water for 32 bits and woot for 64 bit
    constexpr FORCE_INLINE details::HashType<BitNum> ForceConstexprHash(const char* key, size_t len);

    template <>
    constexpr FORCE_INLINE details::HashType<32> Hash<32>(const void* key, size_t len)
    {
        return details::waterhash<false>((const char*)key, static_cast<uint32_t>(len), 0x59B8541C);
    }

    template <>
    constexpr FORCE_INLINE details::HashType<64> Hash<64>(const void* key, size_t len)
    {
        return details::woothash<false>((const char*)key, static_cast<uint64_t>(len), 0x23968DAB);
    }

    template <>
    constexpr FORCE_INLINE details::HashType<32> ForceConstexprHash<32>(const char* key, size_t len)
    {
        return details::waterhash<true>(key, static_cast<uint32_t>(len), 0x59B8541C);
    }

    template <>
    constexpr FORCE_INLINE details::HashType<64> ForceConstexprHash<64>(const char* key, size_t len)
    {
        return details::woothash<true>(key, static_cast<uint64_t>(len), 0x23968DAB);
    }
}
