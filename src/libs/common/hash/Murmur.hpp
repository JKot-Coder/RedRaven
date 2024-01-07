#pragma once

#include <stdint.h>

//-----------------------------------------------------------------------------
// MurmurHash3 by Austin Appleby

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

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline

        template <uint8_t HashBits>
        inline FORCE_INLINE HashType<HashBits> rotl(HashType<HashBits> x, uint8_t r);

        template <>
        inline FORCE_INLINE HashType<32> rotl<32>(HashType<32> x, uint8_t r)
        {
            return _rotl(x, r);
        }

        template <>
        inline FORCE_INLINE HashType<64> rotl<64>(HashType<64> x, uint8_t r)
        {
            return _rotl64(x, r);
        }

#else // defined(_MSC_VER)
#define FORCE_INLINE __attribute__((always_inline))

        template <uint8_t HashBits>
        inline FORCE_INLINE HashType<HashBits> rotl(HashType<HashBits> x, uint8_t r)
        {
            asm("roll %1,%0"
                : "+r"(x)
                : "c"(r));
            return x;
        }
#endif // !defined(_MSC_VER)

        //-----------------------------------------------------------------------------
        // Block read - if your platform needs to do endian-swapping or can only
        // handle aligned reads, do the conversion here

        template <uint8_t HashBits>
        FORCE_INLINE HashType<HashBits> getblock(const HashType<HashBits>* p, int i)
        {
#if true
            return p[i];
#else
            static_assert(false, "Implement endian-swapping");
            return 0;
#endif
        }

        //-----------------------------------------------------------------------------
        // Finalization mix - force all bits of a hash block to avalanche

        template <uint8_t HashBits>
        FORCE_INLINE HashType<HashBits> fmix(HashType<HashBits> h)
        {
            h ^= h >> HashOptions<HashBits>::mixShifts[0];
            h *= HashOptions<HashBits>::mixMuls[0];
            h ^= h >> HashOptions<HashBits>::mixShifts[1];
            h *= HashOptions<HashBits>::mixMuls[1];
            h ^= h >> HashOptions<HashBits>::mixShifts[2];

            return h;
        }

        //-----------------------------------------------------------------------------
        template <uint8_t HashBits>
        HashType<HashBits> hash(const void* key, int len, uint32_t seed);

        template <>
        constexpr HashType<32> hash<32>(const void* key, int len, uint32_t seed)
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
    };
}
//-----------------------------------------------------------------------------
// template <uint8_t HashBits>
// HashType<HashBits>::HashType MurmurHash3(const void* key, int len, uint32_t seed);
