/**
 * @file romutrio.c
 * @brief RomuTrio implementation
 *
 * Based on Mark A. Overton's RomuTrio reference code.
 * Romu source material is licensed under the Apache License, Version 2.0.
 */

#include "prng.h"
#include "prng_common.h"

static struct {
    uint64_t x;
    uint64_t y;
    uint64_t z;
    uint32_t buffer;
    int has_buffered;
} romu_state;

static uint64_t read_le64(const uint8_t* p) {
    uint64_t value = 0;
    for (uint32_t i = 0; i < 8; i++) {
        value |= ((uint64_t)p[i]) << (8U * i);
    }
    return value;
}

static uint64_t rotl64(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64(uint64_t* x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static uint64_t romutrio_next_u64(void) {
    const uint64_t xp = romu_state.x;
    const uint64_t yp = romu_state.y;
    const uint64_t zp = romu_state.z;

    romu_state.x = 15241094284759029579ULL * zp;
    romu_state.y = rotl64(yp - xp, 12);
    romu_state.z = rotl64(zp - yp, 44);

    return xp;
}

static uint32_t romutrio_next_u32(void) {
    if (romu_state.has_buffered) {
        romu_state.has_buffered = 0;
        return romu_state.buffer;
    }

    const uint64_t value = romutrio_next_u64();
    romu_state.buffer = (uint32_t)(value >> 32);
    romu_state.has_buffered = 1;
    return (uint32_t)value;
}

uint32_t getInitSeedBytes(void) {
    return 8;
}

void init(const uint8_t* seed, size_t seed_len) {
    uint64_t seed_value = 0x853c49e6748fea9bULL;

    if (seed_len == 8) {
        seed_value = read_le64(seed);
    }

    uint64_t sm = seed_value;
    romu_state.x = splitmix64(&sm);
    romu_state.y = splitmix64(&sm);
    romu_state.z = splitmix64(&sm);

    if ((romu_state.x | romu_state.y | romu_state.z) == 0) {
        romu_state.x = 0x9e3779b97f4a7c15ULL;
    }

    romu_state.has_buffered = 0;
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, romutrio_next_u32);
}

double getUniformNumber(void) {
    romu_state.has_buffered = 0;
    return u64_to_float64(romutrio_next_u64);
}

#ifdef PRNG_TEST
uint32_t prng_test_next_u32(void) {
    return romutrio_next_u32();
}

uint64_t prng_test_next_u64(void) {
    romu_state.has_buffered = 0;
    return romutrio_next_u64();
}
#endif
