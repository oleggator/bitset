#ifndef BITSET_BITSET_H
#define BITSET_BITSET_H

#include <tarantool/lauxlib.h>

int lbor_in_place(struct lua_State *L);

static const struct luaL_Reg functions[] = {
    {"bor_in_place", lbor_in_place},
    {NULL, NULL}
};

int luaopen_bitset_internal(lua_State *L) {
    luaL_register(L, "bitset_internal", functions);
    return 1;
}

#endif //BITSET_BITSET_H
