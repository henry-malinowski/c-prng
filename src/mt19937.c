/**
 * @file mt19937.c
 * @brief MT19937-64 wrapper implementing unified PRNG interface
 * 
 * Wraps the MT19937-64 reference implementation to provide
 * the unified PRNG interface. MT19937-64 is a 64-bit version
 * of the Mersenne Twister pseudorandom number generator.
 */

#include "prng.h"
#include "prng_common.h"

// Forward declarations for MT19937-64 functions
// These are defined in mt19937-64.c (compiled separately)
extern void init_genrand64(unsigned long long seed);
extern void init_by_array64(unsigned long long init_key[], unsigned long long key_length);
extern unsigned long long genrand64_int64(void);
extern double genrand64_real2(void);

uint32_t getInitSeedBytes(void) {
    return 2496; // 312 × uint64_t = 2496 bytes (same as SFMT-19937)
}

void init(const uint8_t* seed, size_t seed_len) {
    if (seed_len != 2496) {
        // If seed length doesn't match, use a simple seed
        // Convert first 8 bytes to uint64_t
        uint64_t simple_seed = 0;
        if (seed_len >= 8) {
            __builtin_memcpy(&simple_seed, seed, 8);
        } else if (seed_len > 0) {
            __builtin_memcpy(&simple_seed, seed, seed_len);
        }
        init_genrand64(simple_seed);
        return;
    }
    
    // Convert seed bytes to array of uint64_t
    // MT19937-64 expects 312 uint64_t values
    unsigned long long init_key[312];
    __builtin_memcpy(init_key, seed, 2496);
    
    // Initialize using array (312 uint64_t values)
    init_by_array64(init_key, 312);
}

// Helper function to get next u32 from MT19937-64
// Uses caching to split 64-bit values into two 32-bit values
static uint32_t mt19937_next_u32_impl(void) {
    static uint64_t cached_u64 = 0;
    static int use_high = 0;
    
    if (!use_high) {
        cached_u64 = genrand64_int64();
        use_high = 1;
        return (uint32_t)(cached_u64 & 0xFFFFFFFFULL);
    } else {
        use_high = 0;
        return (uint32_t)(cached_u64 >> 32);
    }
}

// Wrapper function to convert genrand64_int64 to uint64_t return type
static uint64_t mt19937_next_u64(void) {
    return (uint64_t)genrand64_int64();
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, mt19937_next_u32_impl);
}

double getUniformNumber(void) {
    return u64_to_float64(mt19937_next_u64);
}
