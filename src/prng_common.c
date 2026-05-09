/**
 * @file prng_common.c
 * @brief Common utilities implementation for PRNG algorithms
 */

#include "prng_common.h"

/**
 * Simple memcpy implementation for use with -nostdlib.
 * This is needed because LTO may expect memcpy to be available.
 * Using unsigned long for size parameter to match common implementations.
 */
void* memcpy(void* dest, const void* src, unsigned long n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    unsigned long i;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

/**
 * Count leading zeros in a 64-bit value.
 * Returns 64 if the value is zero.
 */
static inline uint32_t clz64(uint64_t x) {
    if (x == 0) return 64;
    #if defined(__GNUC__) || defined(__clang__)
    return __builtin_clzll(x);
    #else
    // Fallback implementation
    uint32_t count = 0;
    if ((x & 0xFFFFFFFF00000000ULL) == 0) { count += 32; x <<= 32; }
    if ((x & 0xFFFF000000000000ULL) == 0) { count += 16; x <<= 16; }
    if ((x & 0xFF00000000000000ULL) == 0) { count += 8; x <<= 8; }
    if ((x & 0xF000000000000000ULL) == 0) { count += 4; x <<= 4; }
    if ((x & 0xC000000000000000ULL) == 0) { count += 2; x <<= 2; }
    if ((x & 0x8000000000000000ULL) == 0) { count += 1; }
    return count;
    #endif
}

/**
 * Count leading zeros in a 32-bit value.
 * Returns 32 if the value is zero.
 */
static inline uint32_t clz32(uint32_t x) {
    if (x == 0) return 32;
    #if defined(__GNUC__) || defined(__clang__)
    return __builtin_clz(x);
    #else
    // Fallback implementation
    uint32_t count = 0;
    if ((x & 0xFFFF0000) == 0) { count += 16; x <<= 16; }
    if ((x & 0xFF000000) == 0) { count += 8; x <<= 8; }
    if ((x & 0xF0000000) == 0) { count += 4; x <<= 4; }
    if ((x & 0xC0000000) == 0) { count += 2; x <<= 2; }
    if ((x & 0x80000000) == 0) { count += 1; }
    return count;
    #endif
}

/**
 * Convert random bits to unbiased float32 in [0.0, 1.0).
 * 
 * Method: Use 23 random bits for the mantissa, and count leading zeros
 * for an exponentially biased exponent. This covers every possible
 * float value in [0.0, 1.0).
 * 
 * Based on Zig's std.Random.float implementation.
 */
float u32_to_float32(uint64_t (*next_u64)(void)) {
    // Generate a uniformly random value for the mantissa.
    // Then generate an exponentially biased random value for the exponent.
    const uint64_t rand = next_u64();
    uint32_t rand_lz = clz64(rand);
    
    // If all 41 bits are zero, generate additional random bits
    if (rand_lz >= 41) {
        rand_lz = 41 + clz64(next_u64());
        if (rand_lz == 41 + 64) {
            // It is astronomically unlikely to reach this point.
            const uint64_t extra = next_u64();
            rand_lz += clz32((uint32_t)(extra | 0x7FF));
        }
    }
    
    // Use 23 bits for mantissa (from lower bits of rand)
    const uint32_t mantissa = (uint32_t)(rand & 0x7FFFFF);
    
    // Calculate exponent: 126 - rand_lz, shifted to exponent position
    const uint32_t exponent = (126 - rand_lz) << 23;
    
    // Combine exponent and mantissa, then bit-cast to float
    union {
        uint32_t u;
        float f;
    } converter;
    
    converter.u = exponent | mantissa;
    return converter.f;
}

/**
 * Convert random bits to unbiased float64 in [0.0, 1.0).
 * 
 * Method: Use 52 random bits for the mantissa, and count leading zeros
 * for an exponentially biased exponent. This covers every possible
 * double value in [0.0, 1.0).
 * 
 * Based on Zig's std.Random.float implementation.
 */
double u64_to_float64(uint64_t (*next_u64)(void)) {
    // Generate a uniformly random value for the mantissa.
    // Then generate an exponentially biased random value for the exponent.
    const uint64_t rand = next_u64();
    uint64_t rand_lz = clz64(rand);
    
    // If all 12 bits are zero, generate additional random bits
    if (rand_lz >= 12) {
        rand_lz = 12;
        while (1) {
            // It is astronomically unlikely for this loop to execute more than once.
            const uint64_t addl_rand = next_u64();
            const uint32_t addl_rand_lz = clz64(addl_rand);
            rand_lz += addl_rand_lz;
            if (addl_rand_lz != 64) {
                break;
            }
            if (rand_lz >= 1022) {
                rand_lz = 1022;
                break;
            }
        }
    }
    
    // Use 52 bits for mantissa (from lower bits of rand)
    const uint64_t mantissa = rand & 0xFFFFFFFFFFFFFULL;
    
    // Calculate exponent: 1022 - rand_lz, shifted to exponent position
    const uint64_t exponent = ((uint64_t)(1022 - rand_lz)) << 52;
    
    // Combine exponent and mantissa, then bit-cast to double
    union {
        uint64_t u;
        double d;
    } converter;
    
    converter.u = exponent | mantissa;
    return converter.d;
}

/**
 * Get an unbiased bounded integer in range [min, max] (inclusive).
 *
 * Uses Lemire's nearly-divisionless rejection sampling to avoid modulo bias.
 */
uint32_t getBoundedIntHelper(uint32_t min, uint32_t max, uint32_t (*next_u32)(void)) {
    if (min > max) {
        uint32_t temp = min;
        min = max;
        max = temp;
    }
    
    if (min == max) {
        return min;
    }

    const uint64_t span = (uint64_t)max - (uint64_t)min + 1ULL;

    if (span == (1ULL << 32)) {
        return next_u32();
    }

    const uint32_t range = (uint32_t)span;

    if ((range & (range - 1U)) == 0U) {
        return min + (next_u32() & (range - 1U));
    }

    uint64_t m = (uint64_t)next_u32() * (uint64_t)range;
    uint32_t l = (uint32_t)m;

    if (l < range) {
        const uint32_t t = (uint32_t)(-range) % range;
        while (l < t) {
            m = (uint64_t)next_u32() * (uint64_t)range;
            l = (uint32_t)m;
        }
    }

    return min + (uint32_t)(m >> 32);
}
