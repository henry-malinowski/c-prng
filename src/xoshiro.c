/**
 * @file xoshiro.c
 * @brief Xoshiro256++ implementation
 * 
 * Implements the unified PRNG interface using Xoshiro256++ algorithm.
 * Based on the xoshiro family of random number generators by
 * David Blackman and Sebastiano Vigna.
 * 
 * Reference: https://prng.di.unimi.it/xoshiro256plusplus.c
 */

#include "prng.h"
#include "prng_common.h"

// Xoshiro256++ state (4 × uint64_t = 256 bits)
static uint64_t xoshiro_state[4] = {0, 0, 0, 0};

/**
 * Rotate left operation
 */
static inline uint64_t rotl(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

/**
 * SplitMix64 for seeding (from the xoshiro authors)
 */
static uint64_t splitmix64(uint64_t* x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

/**
 * Generate next 64-bit value from Xoshiro256++ state
 */
static uint64_t xoshiro_next_u64(void) {
    const uint64_t result = rotl(xoshiro_state[0] + xoshiro_state[3], 23) + xoshiro_state[0];
    
    const uint64_t t = xoshiro_state[1] << 17;
    
    xoshiro_state[2] ^= xoshiro_state[0];
    xoshiro_state[3] ^= xoshiro_state[1];
    xoshiro_state[1] ^= xoshiro_state[2];
    xoshiro_state[0] ^= xoshiro_state[3];
    
    xoshiro_state[2] ^= t;
    
    xoshiro_state[3] = rotl(xoshiro_state[3], 45);
    
    return result;
}

/**
 * Generate next 32-bit value from Xoshiro256++ state
 */
static uint32_t xoshiro_next_u32(void) {
    static uint64_t cached_u64 = 0;
    static int use_high = 0;
    
    if (!use_high) {
        cached_u64 = xoshiro_next_u64();
        use_high = 1;
        return (uint32_t)(cached_u64 & 0xFFFFFFFFULL);
    } else {
        use_high = 0;
        return (uint32_t)(cached_u64 >> 32);
    }
}

uint32_t getInitSeedBytes(void) {
    return 8; // u64 = 8 bytes
}

void init(const uint8_t* seed, size_t seed_len) {
    if (seed_len != 8) {
        // Use default seed if length doesn't match
        uint64_t default_seed = 0x853c49e6748fea9bULL;
        uint64_t sm = default_seed;
        xoshiro_state[0] = splitmix64(&sm);
        xoshiro_state[1] = splitmix64(&sm);
        xoshiro_state[2] = splitmix64(&sm);
        xoshiro_state[3] = splitmix64(&sm);
        return;
    }
    
    // Copy seed bytes to uint64_t (little-endian in WASM)
    uint64_t seed_value = 0;
    __builtin_memcpy(&seed_value, seed, 8);
    
    // Use SplitMix64 to initialize state from seed
    // This ensures the state is not everywhere zero
    uint64_t sm = seed_value;
    xoshiro_state[0] = splitmix64(&sm);
    xoshiro_state[1] = splitmix64(&sm);
    xoshiro_state[2] = splitmix64(&sm);
    xoshiro_state[3] = splitmix64(&sm);
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, xoshiro_next_u32);
}

double getUniformNumber(void) {
    return u64_to_float64(xoshiro_next_u64);
}
