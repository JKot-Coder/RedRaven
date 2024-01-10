#pragma once

#include <stdint.h>

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

namespace RR::Common::Hash
{
    namespace Murmur
    {
        template <uint8_t HashBits>
        struct HashOptions;

        template <>
        struct HashOptions<32>
        {
            typedef uint32_t HashType;
            static constexpr HashType mixMuls[2] = { 0x85ebca6b, 0xc2b2ae35 };
            static constexpr HashType mixShifts[3] = { 16, 13, 16 };
        };

        template <>
        struct HashOptions<64>
        {
            typedef uint64_t HashType;
            static constexpr HashType mixMuls[2] = { 0xff51afd7ed558ccd, 0xc4ceb9fe1a85ec53 };
            static constexpr HashType mixShifts[3] = { 33, 33, 33 };
        };

        template <uint8_t HashBits>
        using HashType = typename HashOptions<HashBits>::HashType;

        namespace details
        {
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else // defined(_MSC_VER)
#define FORCE_INLINE __attribute__((always_inline))
#endif // !defined(_MSC_VER)

            template <uint8_t HashBits>
            constexpr inline FORCE_INLINE HashType<HashBits> rotl(HashType<HashBits> x, uint8_t r)
            {
                // Hopefully our compiler is smart enough to optimize for rotl, we can't use _rotl instic
                // or asm("roll %1,%0": "+r"(x): "c"(r)) because we want to get our hash in compile time.
                // TODO: this can be replaced by std::rotr in cpp20
                return (x << (r % std::numeric_limits<HashType<HashBits>>::digits)) |
                       (x >> (std::numeric_limits<HashType<HashBits>>::digits - (r % std::numeric_limits<HashType<HashBits>>::digits)));
            }

            //-----------------------------------------------------------------------------
            // Block read - if your platform needs to do endian-swapping or can only
            // handle aligned reads, do the conversion here

            template <uint8_t HashBits>
            constexpr inline FORCE_INLINE HashType<HashBits> getblock(const HashType<HashBits>* p, int i)
            {
#if true
                // TODO. We cant read block like this while compile time, replace it
                return p[i];
#else
                static_assert(false, "TODO implement endian-swapping");
                return 0;
#endif
            }

            //-----------------------------------------------------------------------------
            // Finalization mix - force all bits of a hash block to avalanche

            template <uint8_t HashBits>
            constexpr inline FORCE_INLINE HashType<HashBits> fmix(HashType<HashBits> h)
            {
                h ^= h >> HashOptions<HashBits>::mixShifts[0];
                h *= HashOptions<HashBits>::mixMuls[0];
                h ^= h >> HashOptions<HashBits>::mixShifts[1];
                h *= HashOptions<HashBits>::mixMuls[1];
                h ^= h >> HashOptions<HashBits>::mixShifts[2];

                return h;
            }
        }

        //-----------------------------------------------------------------------------
        template <uint8_t HashBits>
        constexpr HashType<HashBits> Hash(const char* const key, size_t len, uint32_t seed = 0);

        template <>
        constexpr HashType<32> Hash<32>(const char* const key, size_t len, uint32_t seed)
        {
            const uint8_t* data = (const uint8_t*)key;
            const int nblocks = len / 4;

            uint32_t h1 = seed;

            const uint32_t c1 = 0xcc9e2d51;
            const uint32_t c2 = 0x1b873593;

            //----------
            // body

            const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);

            for (int i = -nblocks; i; i++)
            {
                uint32_t k1 = getblock<32>(blocks, i);

                k1 *= c1;
                k1 = rotl<32>(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = rotl<32>(h1, 13);
                h1 = h1 * 5 + 0xe6546b64;
            }

            //----------
            // tail

            const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);

            uint32_t k1 = 0;

            switch (len & 3)
            {
                case 3: k1 ^= tail[2] << 16;
                case 2: k1 ^= tail[1] << 8;
                case 1:
                    k1 ^= tail[0];
                    k1 *= c1;
                    k1 = rotl<32>(k1, 15);
                    k1 *= c2;
                    h1 ^= k1;
            };

            //----------
            // finalization

            h1 ^= len;
            h1 = fmix<32>(h1);

            return h1;
        }
        //-----------------------------------------------------------------------------
        template <uint8_t HashBits>
        constexpr HashType<HashBits> Hash(const std::string_view string, uint32_t seed = 0)
        {
            std::ignore = seed;
            return Hash<HashBits>(string.data(), string.length(), 0);
        }
    };
}
