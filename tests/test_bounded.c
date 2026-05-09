#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "prng_common.h"

static uint64_t stub_state;
static const uint32_t* queued_values;
static size_t queued_len;
static size_t queued_pos;

static uint64_t splitmix64_next(void) {
    uint64_t z = (stub_state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static uint32_t next_stub_u32(void) {
    if (queued_pos < queued_len) {
        return queued_values[queued_pos++];
    }
    return (uint32_t)splitmix64_next();
}

static void require(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "bounded: %s\n", message);
        exit(1);
    }
}

static void queue_values(const uint32_t* values, size_t len) {
    queued_values = values;
    queued_len = len;
    queued_pos = 0;
}

static void test_edges(void) {
    const uint32_t full_span[] = {0xdeadbeefU};
    queue_values(full_span, 1);
    require(getBoundedIntHelper(0, UINT32_MAX, next_stub_u32) == 0xdeadbeefU,
            "full uint32 span should return raw sample");

    const uint32_t power_two[] = {0xffffffffU};
    queue_values(power_two, 1);
    require(getBoundedIntHelper(10, 25, next_stub_u32) == 25,
            "power-of-two span should use mask");

    const uint32_t swap_values[] = {0};
    queue_values(swap_values, 1);
    require(getBoundedIntHelper(9, 8, next_stub_u32) == 8,
            "min > max should preserve swap behavior");

    queued_pos = queued_len = 0;
    require(getBoundedIntHelper(42, 42, next_stub_u32) == 42,
            "single-value range should return min");
}

static void test_distribution(uint32_t range, uint64_t samples) {
    uint32_t counts[128] = {0};
    const double expected = (double)samples / (double)range;
    double chi_square = 0.0;

    require(range <= 128, "distribution range exceeds counts buffer");

    stub_state = 0x6a09e667f3bcc909ULL ^ range;
    queued_pos = queued_len = 0;

    for (uint64_t i = 0; i < samples; i++) {
        const uint32_t value = getBoundedIntHelper(0, range - 1U, next_stub_u32);
        require(value < range, "sample outside requested range");
        counts[value]++;
    }

    for (uint32_t i = 0; i < range; i++) {
        const double delta = (double)counts[i] - expected;
        chi_square += (delta * delta) / expected;
    }

    if (chi_square > (double)range * 3.0) {
        fprintf(stderr,
                "bounded: chi-square too high for range %u: %.3f\n",
                range,
                chi_square);
        exit(1);
    }
}

int main(void) {
    test_edges();
    test_distribution(7, 10000000ULL);
    test_distribution(20, 10000000ULL);
    test_distribution(100, 10000000ULL);
    return 0;
}
