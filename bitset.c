#include "bitset.h"
#include <tarantool/module.h>
#include "msgpuck/msgpuck.h"

void bor(uint8_t *dst, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dst[i] |= src[i];
    }
}

int lbor_in_place(struct lua_State *L) {
    uint32_t binary_type = luaL_ctypeid(L, "uint8_t *");

    uint8_t *dst = luaL_checkcdata(L, 1, &binary_type);
    uint8_t *src = luaL_checkcdata(L, 2, &binary_type);
    uint64_t len = luaL_checkuint64(L, 3);

    bor(dst, src, len);

    return 0;
}

int lbor_tuple_in_place(struct lua_State *L) {
    uint32_t binary_type = luaL_ctypeid(L, "uint8_t *");

    uint8_t *dst = luaL_checkcdata(L, 1, &binary_type);
    uint64_t len = luaL_checkuint64(L, 2);
    box_tuple_t *tuple = luaT_istuple(L, 3);
    uint64_t field_no = luaL_checkuint64(L, 4);

    box_tuple_ref(tuple);
    const uint8_t *src = (const uint8_t *) box_tuple_field(tuple, field_no);
    mp_decode_binl((const char **) &src); // decode binary length

    bor(dst, src, len);

    box_tuple_unref(tuple);
    return 0;
}

int lto_tuple(struct lua_State *L) {
    uint32_t binary_type = luaL_ctypeid(L, "uint8_t *");

    uint8_t *body = luaL_checkcdata(L, 1, &binary_type);
    uint64_t len = luaL_checkuint64(L, 2);

    char *tuple_body = calloc(len + 5, sizeof(char));
    char *tuple_body_cursor = tuple_body;
    tuple_body_cursor = mp_encode_binl(tuple_body_cursor, len);

    uint32_t bin_header_len = tuple_body_cursor - tuple_body;
    memcpy(tuple_body_cursor, body, len);

    box_tuple_t *tuple = box_tuple_new(box_tuple_format_default(),
                                       tuple_body,
                                       tuple_body + bin_header_len + len);
    free(tuple_body);
    luaT_pushtuple(L, tuple);

    return 1;
}
