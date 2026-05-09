#include <stdint.h>
#include <stdio.h>

#include "prng.h"
#include "test_support.h"

#ifndef VECTOR_COUNT
#define VECTOR_COUNT 1000
#endif

uint32_t prng_test_next_u32(void);

int main(void) {
    uint8_t seed[2496];
    const uint32_t seed_len = getInitSeedBytes();

    test_fill_seed(seed, seed_len, 0);
    init(seed, seed_len);

    for (uint32_t i = 0; i < VECTOR_COUNT; i++) {
        printf("%08x\n", prng_test_next_u32());
    }

    return 0;
}
