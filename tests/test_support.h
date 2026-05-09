#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <stddef.h>
#include <stdint.h>

static uint64_t test_splitmix64(uint64_t* x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static void test_write_le64(uint8_t* dest, uint64_t value) {
    for (uint32_t i = 0; i < 8; i++) {
        dest[i] = (uint8_t)(value >> (8U * i));
    }
}

static void test_fill_seed(uint8_t* seed, size_t seed_len, uint64_t salt) {
    if (seed_len == 8) {
        test_write_le64(seed, 0x0123456789abcdefULL ^ salt);
        return;
    }

    if (seed_len == 32) {
        test_write_le64(seed + 0, 0x0123456789abcdefULL ^ salt);
        test_write_le64(seed + 8, 0xfedcba9876543210ULL + salt);
        test_write_le64(seed + 16, 0x0f1e2d3c4b5a6978ULL ^ (salt << 1));
        test_write_le64(seed + 24, 0x8877665544332211ULL + (salt << 1));
        return;
    }

    uint64_t sm = 0x243f6a8885a308d3ULL ^ salt;
    for (size_t i = 0; i < seed_len; i += 8) {
        const uint64_t value = test_splitmix64(&sm);
        const size_t remaining = seed_len - i;
        const size_t n = remaining < 8 ? remaining : 8;
        for (size_t j = 0; j < n; j++) {
            seed[i + j] = (uint8_t)(value >> (8U * j));
        }
    }
}

#endif
