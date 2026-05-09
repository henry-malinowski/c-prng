/**
 * @file well19937a.c
 * @brief WELL19937a implementation
 *
 * Adapted from WELLRNG19937a.c by Francois Panneton, Pierre L'Ecuyer,
 * and Makoto Matsumoto.
 *
 * Upstream notice: the reference code can be used freely for personal,
 * academic, or non-commercial purposes. For commercial purposes, contact
 * Pierre L'Ecuyer. The code can also be used under GPL version 3 or later.
 */

#include "prng.h"
#include "prng_common.h"

#define WELL_W 32
#define WELL_R 624
#define WELL_P 31
#define WELL_MASKU (0xffffffffU >> (WELL_W - WELL_P))
#define WELL_MASKL (~WELL_MASKU)
#define WELL_M1 70
#define WELL_M2 179
#define WELL_M3 449

static uint32_t well_state[WELL_R];
static uint32_t well_i;

static uint64_t splitmix64(uint64_t* x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static uint32_t read_le32(const uint8_t* p) {
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint32_t well_next_u32(void) {
    const uint32_t i0 = well_i;
    const uint32_t im1 = (well_i + WELL_R - 1U) % WELL_R;
    const uint32_t im2 = (well_i + WELL_R - 2U) % WELL_R;
    const uint32_t im1_offset = (well_i + WELL_M1) % WELL_R;
    const uint32_t im2_offset = (well_i + WELL_M2) % WELL_R;
    const uint32_t im3_offset = (well_i + WELL_M3) % WELL_R;

    const uint32_t z0 = (well_state[im1] & WELL_MASKL) |
                        (well_state[im2] & WELL_MASKU);
    const uint32_t z1 = (well_state[i0] ^ (well_state[i0] << 25)) ^
                        (well_state[im1_offset] ^ (well_state[im1_offset] >> 27));
    const uint32_t z2 = (well_state[im2_offset] >> 9) ^
                        (well_state[im3_offset] ^ (well_state[im3_offset] >> 1));

    well_state[i0] = z1 ^ z2;
    well_state[im1] = z0 ^
                      (z1 ^ (z1 << 9)) ^
                      (z2 ^ (z2 << 21)) ^
                      (well_state[i0] ^ (well_state[i0] >> 21));
    well_i = im1;

    return well_state[well_i];
}

static uint64_t well_next_u64(void) {
    const uint64_t low = (uint64_t)well_next_u32();
    const uint64_t high = (uint64_t)well_next_u32();
    return (high << 32) | low;
}

uint32_t getInitSeedBytes(void) {
    return WELL_R * 4U;
}

void init(const uint8_t* seed, size_t seed_len) {
    uint32_t nonzero = 0;
    well_i = 0;

    if (seed_len == WELL_R * 4U) {
        for (uint32_t i = 0; i < WELL_R; i++) {
            well_state[i] = read_le32(seed + (i * 4U));
            nonzero |= well_state[i];
        }
    } else {
        uint64_t sm = 0x853c49e6748fea9bULL;
        if (seed_len > 0) {
            sm = 0;
            const size_t copy_len = seed_len < 8 ? seed_len : 8;
            for (size_t i = 0; i < copy_len; i++) {
                sm |= ((uint64_t)seed[i]) << (8U * i);
            }
        }

        for (uint32_t i = 0; i < WELL_R; i += 2) {
            const uint64_t value = splitmix64(&sm);
            well_state[i] = (uint32_t)value;
            if (i + 1U < WELL_R) {
                well_state[i + 1U] = (uint32_t)(value >> 32);
            }
            nonzero |= well_state[i];
            if (i + 1U < WELL_R) {
                nonzero |= well_state[i + 1U];
            }
        }
    }

    if (nonzero == 0) {
        well_state[0] = 0x80000000U;
    }
}

uint32_t getBoundedInt(uint32_t min, uint32_t max) {
    return getBoundedIntHelper(min, max, well_next_u32);
}

double getUniformNumber(void) {
    return (double)u32_to_float32(well_next_u64);
}

#ifdef PRNG_TEST
uint32_t prng_test_next_u32(void) {
    return well_next_u32();
}
#endif
