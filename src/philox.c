/**
 * @file philox.c
 * @brief Philox-4x32-10 wrapper implementing unified PRNG interface
 * 
 * Wraps the Random123 Philox reference implementation to provide
 * the unified PRNG interface. Philox is a counter-based PRNG.
 */

#include "prng.h"
#include "prng_common.h"

// Include Philox header from Random123
#include "../vendor/random123/Random123/philox.h"

// Philox state: counter and key
static philox4x32_ctr_t philox_ctr = {{0, 0, 0, 0}};
static philox4x32_key_t philox_key = {{0, 0}};

// Cache for generated values (Philox generates 4 u32 at once)
static uint32_t philox_cache[4] = {0, 0, 0, 0};
static int philox_cache_idx = 4; // Start with empty cache

/**
 * Generate next 32-bit value from Philox.
 * Uses caching to split 4×32 output into individual u32 values.
 */
static uint32_t philox_next_u32(void) {
    if (philox_cache_idx >= 4) {
        // Generate new block of 4 u32 values
        philox4x32_ctr_t out = philox4x32(philox_ctr, philox_key);
        philox_cache[0] = out.v[0];
        philox_cache[1] = out.v[1];
        philox_cache[2] = out.v[2];
        philox_cache[3] = out.v[3];
        philox_cache_idx = 0;
        
        // Increment counter (128-bit little-endian)
        philox_ctr.v[0]++;
        if (philox_ctr.v[0] == 0) {
            philox_ctr.v[1]++;
            if (philox_ctr.v[1] == 0) {
                philox_ctr.v[2]++;
                if (philox_ctr.v[2] == 0) {
                    philox_ctr.v[3]++;
                }
            }
        }
    }
    
    return philox_cache[philox_cache_idx++];
}

/**
 * Generate next 64-bit value from Philox.
 * Combines two u32 values to form u64.
 */
static uint64_t philox_next_u64(void) {
    uint64_t low = (uint64_t)philox_next_u32();
    uint64_t high = (uint64_t)philox_next_u32();
    return (high << 32) | low;
}

uint32_t getInitSeedBytes(void) {
    return 8; // u64 = 8 bytes (used for key initialization)
}

void init(const uint8_t* seed, size_t seed_len) {
    if (seed_len != 8) {
        // Use default seed if length doesn't match
        philox_key.v[0] = 0;
        philox_key.v[1] = 0;
    } else {
        // Copy seed bytes to key (2 × uint32_t)
        uint64_t seed_value = 0;
        __builtin_memcpy(&seed_value, seed, 8);
        
        // Split 64-bit seed into two 32-bit key values
        philox_key.v[0] = (uint32_t)(seed_value & 0xFFFFFFFFULL);
        philox_key.v[1] = (uint32_t)(seed_value >> 32);
    }
    
    // Initialize counter to zero
    philox_ctr.v[0] = 0;
    philox_ctr.v[1] = 0;
    philox_ctr.v[2] = 0;
    philox_ctr.v[3] = 0;
    
    // Reset cache
    philox_cache_idx = 4;
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, philox_next_u32);
}

double getUniformNumber(void) {
    return (double)u32_to_float32(philox_next_u64);
}
