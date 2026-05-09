CC ?= cc
CFLAGS ?= -std=c99 -O2 -Wall -Wextra
CPPFLAGS ?= -Iinclude -Isrc

COMMON = src/prng_common.c
BUILD_DIR = build
VECTORS = \
	tests/vectors/philox_u32.txt \
	tests/vectors/xoshiro_u32.txt \
	tests/vectors/well19937a_u32.txt \
	tests/vectors/pcg_u32.txt \
	tests/vectors/romutrio_u32.txt

.PHONY: test vectors clean

test: \
	$(BUILD_DIR)/test_bounded \
	$(BUILD_DIR)/test_philox \
	$(BUILD_DIR)/test_xoshiro \
	$(BUILD_DIR)/test_well19937a \
	$(BUILD_DIR)/test_pcg \
	$(BUILD_DIR)/test_romutrio
	$(BUILD_DIR)/test_bounded
	$(BUILD_DIR)/test_philox
	$(BUILD_DIR)/test_xoshiro
	$(BUILD_DIR)/test_well19937a
	$(BUILD_DIR)/test_pcg
	$(BUILD_DIR)/test_romutrio

vectors: $(VECTORS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

tests/vectors:
	mkdir -p tests/vectors

$(BUILD_DIR)/test_bounded: tests/test_bounded.c $(COMMON) src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $< $(COMMON) -lm -o $@

$(BUILD_DIR)/test_philox: tests/test_prng.c tests/test_support.h src/philox.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST -DALGO_NAME=\"philox\" tests/test_prng.c src/philox.c $(COMMON) -o $@

$(BUILD_DIR)/test_xoshiro: tests/test_prng.c tests/test_support.h src/xoshiro.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST -DEXPECT_U64_BUFFER -DALGO_NAME=\"xoshiro\" tests/test_prng.c src/xoshiro.c $(COMMON) -o $@

$(BUILD_DIR)/test_well19937a: tests/test_prng.c tests/test_support.h src/well19937a.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST -DALGO_NAME=\"well19937a\" tests/test_prng.c src/well19937a.c $(COMMON) -o $@

$(BUILD_DIR)/test_pcg: tests/test_prng.c tests/test_support.h src/pcg.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST -DEXPECT_U64_BUFFER -DALGO_NAME=\"pcg\" tests/test_prng.c src/pcg.c $(COMMON) -o $@

$(BUILD_DIR)/test_romutrio: tests/test_prng.c tests/test_support.h src/romutrio.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST -DEXPECT_U64_BUFFER -DALGO_NAME=\"romutrio\" tests/test_prng.c src/romutrio.c $(COMMON) -o $@

$(BUILD_DIR)/dump_philox: tests/dump_vector.c tests/test_support.h src/philox.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST tests/dump_vector.c src/philox.c $(COMMON) -o $@

$(BUILD_DIR)/dump_xoshiro: tests/dump_vector.c tests/test_support.h src/xoshiro.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST tests/dump_vector.c src/xoshiro.c $(COMMON) -o $@

$(BUILD_DIR)/dump_well19937a: tests/dump_vector.c tests/test_support.h src/well19937a.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST tests/dump_vector.c src/well19937a.c $(COMMON) -o $@

$(BUILD_DIR)/dump_pcg: tests/dump_vector.c tests/test_support.h src/pcg.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST tests/dump_vector.c src/pcg.c $(COMMON) -o $@

$(BUILD_DIR)/dump_romutrio: tests/dump_vector.c tests/test_support.h src/romutrio.c $(COMMON) include/prng.h src/prng_common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DPRNG_TEST tests/dump_vector.c src/romutrio.c $(COMMON) -o $@

tests/vectors/philox_u32.txt: $(BUILD_DIR)/dump_philox | tests/vectors
	$(BUILD_DIR)/dump_philox > $@

tests/vectors/xoshiro_u32.txt: $(BUILD_DIR)/dump_xoshiro | tests/vectors
	$(BUILD_DIR)/dump_xoshiro > $@

tests/vectors/well19937a_u32.txt: $(BUILD_DIR)/dump_well19937a | tests/vectors
	$(BUILD_DIR)/dump_well19937a > $@

tests/vectors/pcg_u32.txt: $(BUILD_DIR)/dump_pcg | tests/vectors
	$(BUILD_DIR)/dump_pcg > $@

tests/vectors/romutrio_u32.txt: $(BUILD_DIR)/dump_romutrio | tests/vectors
	$(BUILD_DIR)/dump_romutrio > $@

clean:
	rm -rf $(BUILD_DIR)
