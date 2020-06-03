#ifndef BITSET_BITSET_H
#define BITSET_BITSET_H

#include <tarantool/lauxlib.h>

typedef struct {
    uint64_t size;
    uint64_t bin_header_size;
    uint8_t msgpack[1]; // variable part
} bitset_t;

int lnew(lua_State *L);

int lnew_from_string(lua_State *L);

int lnew_from_tuple(lua_State *L);

int lbor_in_place(lua_State *L);

int lbor_tuple_in_place(lua_State *L);

int lto_tuple(lua_State *L);

int lto_string(lua_State *L);

static const struct luaL_Reg functions[] = {
    {"new",                lnew},
    {"new_from_string",    lnew_from_string},
    {"new_from_tuple",     lnew_from_tuple},
    {"bor_in_place",       lbor_in_place},
    {"bor_tuple_in_place", lbor_tuple_in_place},
    {"to_tuple",           lto_tuple},
    {"to_string",          lto_string},
    {NULL, NULL},
};

int luaopen_bitset(lua_State *L) {
    luaL_register(L, "bitset", functions);
    return 1;
}

#endif //BITSET_BITSET_H
