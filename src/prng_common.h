/**
 * @file prng_common.h
 * @brief Common utilities for PRNG implementations
 * 
 * Shared functions for unbiased float conversion and bounded integer generation.
 */

#ifndef PRNG_COMMON_H
#define PRNG_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert random bits to unbiased float32 in [0.0, 1.0).
 * Uses exponentially biased distribution to cover all possible float values.
 * Based on Zig's std.Random.float implementation.
 * @param next_u64 Function pointer to generate next uint64_t
 * @return Unbiased float32 in [0.0, 1.0)
 */
float u32_to_float32(uint64_t (*next_u64)(void));

/**
 * Convert random bits to unbiased float64 in [0.0, 1.0).
 * Uses exponentially biased distribution to cover all possible float values.
 * Based on Zig's std.Random.float implementation.
 * @param next_u64 Function pointer to generate next uint64_t
 * @return Unbiased float64 in [0.0, 1.0)
 */
double u64_to_float64(uint64_t (*next_u64)(void));

/**
 * Get an unbiased bounded integer in range [min, max] (inclusive).
 * Uses Lemire's nearly-divisionless rejection sampling to avoid modulo bias.
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @param next_u32 Function pointer to generate next uint32_t
 * @return Unbiased random integer in [min, max]
 */
uint32_t getBoundedIntHelper(uint32_t min, uint32_t max, uint32_t (*next_u32)(void));

#ifdef __cplusplus
}
#endif

#endif /* PRNG_COMMON_H */
