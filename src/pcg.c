/**
 * @file pcg.c
 * @brief PCG (Permuted Congruential Generator) implementation
 * 
 * Implements the unified PRNG interface using PCG algorithm.
 * Based on the PCG family of random number generators.
 */

#include "prng.h"
#include "prng_common.h"

// PCG state structure
static struct {
    uint64_t state;
    uint64_t inc;
} pcg_state;

// PCG constants
#define PCG_MULTIPLIER 6364136223846793005ULL
#define PCG_DEFAULT_INC 1442695040888963407ULL

/**
 * Generate next 32-bit random value from PCG state.
 * Uses XSH-RR (XorShift and Rotate) output function.
 */
static uint32_t pcg_next_u32(void) {
    uint64_t oldstate = pcg_state.state;
    
    // Advance state: LCG step
    pcg_state.state = oldstate * PCG_MULTIPLIER + pcg_state.inc;
    
    // XSH-RR output function: XorShift high bits, then Rotate
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

/**
 * Generate next 64-bit random value from PCG state.
 */
static uint64_t pcg_next_u64(void) {
    uint64_t low = (uint64_t)pcg_next_u32();
    uint64_t high = (uint64_t)pcg_next_u32();
    return (high << 32) | low;
}

uint32_t getInitSeedBytes(void) {
    return 8; // u64 = 8 bytes
}

void init(const uint8_t* seed, size_t seed_len) {
    if (seed_len != 8) {
        // In WASM, we can't panic, so we'll use a default seed
        pcg_state.state = 0x853c49e6748fea9bULL;
        pcg_state.inc = PCG_DEFAULT_INC;
        return;
    }
    
    // Copy seed bytes to uint64_t (little-endian in WASM)
    uint64_t seed_value = 0;
    __builtin_memcpy(&seed_value, seed, 8);
    
    // Initialize PCG state
    // Use the seed as the initial state, and a fixed increment
    pcg_state.state = 0;
    pcg_state.inc = (seed_value << 1) | 1; // Ensure odd increment
    pcg_next_u32(); // Advance once to mix seed
    pcg_state.state += seed_value;
    pcg_next_u32(); // Advance again
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, pcg_next_u32);
}

double getUniformNumber(void) {
    return (double)u32_to_float32(pcg_next_u64);
}
