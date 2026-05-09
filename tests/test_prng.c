#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "prng.h"
#include "test_support.h"

#ifndef ALGO_NAME
#error "ALGO_NAME must be defined"
#endif

#define VECTOR_COUNT 1000

uint32_t prng_test_next_u32(void);

#ifdef EXPECT_U64_BUFFER
uint64_t prng_test_next_u64(void);
#endif

static void require(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "%s: %s\n", ALGO_NAME, message);
        exit(1);
    }
}

static void init_with_salt(uint64_t salt) {
    uint8_t seed[2496];
    const uint32_t seed_len = getInitSeedBytes();
    require(seed_len <= sizeof(seed), "seed length exceeds test buffer");
    test_fill_seed(seed, seed_len, salt);
    init(seed, seed_len);
}

static void test_vectors(void) {
    char path[128];
    snprintf(path, sizeof(path), "tests/vectors/%s_u32.txt", ALGO_NAME);

    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "%s: could not open %s\n", ALGO_NAME, path);
        exit(1);
    }

    init_with_salt(0);

    for (uint32_t i = 0; i < VECTOR_COUNT; i++) {
        unsigned int expected;
        if (fscanf(f, "%x", &expected) != 1) {
            fprintf(stderr, "%s: missing vector at index %u\n", ALGO_NAME, i);
            fclose(f);
            exit(1);
        }

        const uint32_t actual = prng_test_next_u32();
        if (actual != (uint32_t)expected) {
            fprintf(stderr,
                    "%s: vector %u expected %08x got %08x\n",
                    ALGO_NAME,
                    i,
                    (uint32_t)expected,
                    actual);
            fclose(f);
            exit(1);
        }
    }

    fclose(f);
}

#ifdef EXPECT_U64_BUFFER
static void test_buffer_halves(void) {
    init_with_salt(0);
    const uint64_t native = prng_test_next_u64();

    init_with_salt(0);
    const uint32_t low = prng_test_next_u32();
    const uint32_t high = prng_test_next_u32();

    require(low == (uint32_t)native, "next_u32 did not return low half first");
    require(high == (uint32_t)(native >> 32), "next_u32 did not buffer high half");
}

static void test_reseed_clears_buffer(void) {
    init_with_salt(0);
    (void)prng_test_next_u32();

    init_with_salt(0xfeedfacecafebeefULL);
    const uint32_t after_reseed = prng_test_next_u32();

    init_with_salt(0xfeedfacecafebeefULL);
    const uint64_t native = prng_test_next_u64();

    require(after_reseed == (uint32_t)native, "reseed did not clear buffered half");
}
#endif

int main(void) {
    test_vectors();

#ifdef EXPECT_U64_BUFFER
    test_buffer_halves();
    test_reseed_clears_buffer();
#endif

    return 0;
}
