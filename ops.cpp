#include "ops.h"

void bit_or(uint8_t *dst, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dst[i] |= src[i];
    }
}

void bit_and(uint8_t *dst, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dst[i] &= src[i];
    }
}

void bit_xor(uint8_t *dst, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dst[i] ^= src[i];
    }
}
