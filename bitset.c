#include "bitset.h"

#include <tarantool/module.h>
#include <msgpuck.h>

void merge(uint8_t *dst, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dst[i] |= src[i];
    }
}

int lmerge(struct lua_State *L) {
    uint8_t *dst = (uint8_t *) luaL_checkuint64(L, 1);
    uint8_t *src = (uint8_t *) luaL_checkuint64(L, 2);
    uint64_t len = luaL_checkuint64(L, 3);
    merge(dst, src, len);

    return 0;
}

void merge_segment(uint8_t *dst, box_tuple_t *segment, uint32_t len) {
    box_tuple_ref(segment);

    const char *src = box_tuple_field(segment, 1);
    mp_decode_binl(&src);

    merge(dst, src, len);

    box_tuple_unref(segment);
}

size_t get_header_len(uint32_t len) {
    if (len <= UINT8_MAX) {
        return 2;
    }
    if (len <= UINT16_MAX) {
        return 3;
    }
    return 5;
}

box_tuple_t *merge_segments(uint32_t *ids, uint32_t len) {
    const int key_msgpack_len = 10;  // max uint64 msgpack size + array header
    char key_msgpack[key_msgpack_len];
    char *arr = mp_encode_array(key_msgpack, 1);
    mp_encode_uint(arr, ids[0]);

    box_tuple_t *result;
    int err = box_index_get(512, 0, key_msgpack, key_msgpack + key_msgpack_len,
                            &result);
    if (err != 0) {
        fprintf(stderr, "%s\n", box_error_message(box_error_last()));
        return NULL;
    }
    if (result == NULL) {
        fprintf(stderr, "%s\n", "not found");
        return NULL;
    }

    box_tuple_ref(result);
    const char *bitset_msgpack = box_tuple_field(result, 1);
    uint32_t bitset_len = mp_decode_binl(&bitset_msgpack);

    size_t bin_header_len = get_header_len(bitset_len);

    char *merged_bitset_msgpack = malloc(bin_header_len + bitset_len);
    char *merged_bitset = merged_bitset_msgpack + bin_header_len;
    memcpy(merged_bitset_msgpack, bitset_msgpack - bin_header_len, bitset_len + bin_header_len);
    box_tuple_unref(result);

    for (uint32_t i = 1; i < len; ++i) {
        mp_encode_uint(mp_encode_array(key_msgpack, 1), ids[0]);

        int err = box_index_get(512, 0, key_msgpack,
                                key_msgpack + key_msgpack_len, &result);
        if (err != 0) {
            fprintf(stderr, "%s\n", box_error_message(box_error_last()));
            return NULL;
        }
        if (result == NULL) {
            fprintf(stderr, "%s\n", "not found");
            return NULL;
        }

        box_tuple_ref(result);
        const char *bitset = box_tuple_field(result, 1) + bin_header_len;
        merge(merged_bitset, bitset, bitset_len);
        box_tuple_unref(result);
    }

    box_tuple_t *tuple = box_tuple_new(
        box_tuple_format_default(), merged_bitset_msgpack,
        merged_bitset_msgpack + bin_header_len + bitset_len);
    free(merged_bitset_msgpack);

    return tuple;
}



