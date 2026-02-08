/**
 * @file prng.h
 * @brief Unified interface contract for all PRNG algorithms
 * 
 * All PRNG algorithms must implement this interface. This allows
 * algorithms to be swapped at compile/link time.
 */

#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the number of bytes required for seeding this PRNG.
 * @return Number of seed bytes required
 */
uint32_t getInitSeedBytes(void);

/**
 * Initialize the PRNG with a seed array.
 * @param seed Pointer to uint8_t array containing seed data
 * @param seed_len Number of bytes (must equal getInitSeedBytes())
 */
void init(const uint8_t* seed, size_t seed_len);

/**
 * Get a random integer in the range [min, max] (inclusive).
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random integer in [min, max]
 */
uint32_t getBoundedInt(uint32_t min, uint32_t max);

/**
 * Get a uniform random number in [0.0, 1.0).
 *
 * Each algorithm uses its natural conversion path internally:
 * 32-bit generators convert u32 -> float -> double,
 * 64-bit generators convert u64 -> double directly.
 * @return Random double in [0.0, 1.0)
 */
double getUniformNumber(void);

#ifdef __cplusplus
}
#endif

#endif /* PRNG_H */
