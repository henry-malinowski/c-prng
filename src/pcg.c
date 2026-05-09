/**
 * @file pcg.c
 * @brief PCG64 DXSM implementation
 *
 * Implements PCG cm_setseq_dxsm_128_64 using the unified PRNG interface.
 *
 * References:
 * - PCG C++ reference implementation by Melissa O'Neill and contributors,
 *   SPDX-License-Identifier: Apache-2.0 OR MIT.
 * - pcg64_dxsm.c by Tony Finch, SPDX-License-Identifier: 0BSD OR MIT-0.
 */

#include "prng.h"
#include "prng_common.h"

#if !defined(__SIZEOF_INT128__)
#error "PCG64 DXSM requires compiler support for __uint128_t"
#endif

typedef __uint128_t pcg128_t;

static struct {
    pcg128_t state;
    pcg128_t inc;
    uint32_t buffer;
    int has_buffered;
} pcg_state;

#define PCG64_DXSM_MULTIPLIER 0xda942042e4dd58b5ULL

static uint64_t read_le64(const uint8_t* p) {
    uint64_t value = 0;
    for (uint32_t i = 0; i < 8; i++) {
        value |= ((uint64_t)p[i]) << (8U * i);
    }
    return value;
}

static pcg128_t read_le128(const uint8_t* p) {
    const pcg128_t low = (pcg128_t)read_le64(p);
    const pcg128_t high = (pcg128_t)read_le64(p + 8);
    return low | (high << 64);
}

static uint64_t pcg_next_u64(void) {
    const uint64_t mul = PCG64_DXSM_MULTIPLIER;
    const pcg128_t oldstate = pcg_state.state;

    pcg_state.state = oldstate * (pcg128_t)mul + pcg_state.inc;

    uint64_t hi = (uint64_t)(oldstate >> 64);
    const uint64_t lo = (uint64_t)oldstate | 1ULL;

    hi ^= hi >> 32;
    hi *= mul;
    hi ^= hi >> 48;
    hi *= lo;
    return hi;
}

static uint32_t pcg_next_u32(void) {
    if (pcg_state.has_buffered) {
        pcg_state.has_buffered = 0;
        return pcg_state.buffer;
    }

    const uint64_t value = pcg_next_u64();
    pcg_state.buffer = (uint32_t)(value >> 32);
    pcg_state.has_buffered = 1;
    return (uint32_t)value;
}

static void pcg_seed(pcg128_t initstate, pcg128_t initseq) {
    pcg_state.state = 0;
    pcg_state.inc = (initseq << 1) | 1U;
    pcg_state.has_buffered = 0;
    (void)pcg_next_u64();
    pcg_state.state += initstate;
    (void)pcg_next_u64();
    pcg_state.has_buffered = 0;
}

uint32_t getInitSeedBytes(void) {
    return 32;
}

void init(const uint8_t* seed, size_t seed_len) {
    pcg128_t initstate;
    pcg128_t initseq;

    if (seed_len == 32) {
        initstate = read_le128(seed);
        initseq = read_le128(seed + 16);
    } else {
        initstate = (((pcg128_t)0x853c49e6748fea9bULL) << 64) |
                    (pcg128_t)0xda3e39cb94b95bdbULL;
        initseq = (((pcg128_t)0x5851f42d4c957f2dULL) << 64) |
                  (pcg128_t)0x14057b7ef767814fULL;
    }

    pcg_seed(initstate, initseq);
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, pcg_next_u32);
}

double getUniformNumber(void) {
    pcg_state.has_buffered = 0;
    return u64_to_float64(pcg_next_u64);
}

#ifdef PRNG_TEST
uint32_t prng_test_next_u32(void) {
    return pcg_next_u32();
}

uint64_t prng_test_next_u64(void) {
    pcg_state.has_buffered = 0;
    return pcg_next_u64();
}
#endif
