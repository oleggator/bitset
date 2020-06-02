#include "bitset.h"
#include <tarantool/module.h>

void bor_in_place(uint8_t *dst, const uint8_t *src, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        dst[i] |= src[i];
    }
}

int lbor_in_place(struct lua_State *L) {
    uint32_t void_ptr_type = luaL_ctypeid(L, "uint8_t *");
    uint8_t *dst = luaL_checkcdata(L, 1, &void_ptr_type);
    uint8_t *src = luaL_checkcdata(L, 2, &void_ptr_type);
    uint64_t len = luaL_checkuint64(L, 3);
    bor_in_place(dst, src, len);

    return 0;
}
