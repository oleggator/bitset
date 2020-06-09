#ifndef BITSET_BITSET_H
#define BITSET_BITSET_H

#include "ops.h"

extern "C" {
#include <tarantool/lauxlib.h>
int luaopen_bitset(lua_State *L);
}

typedef struct {
    uint64_t size;
    uint64_t bin_header_size;
    uint8_t msgpack[1]; // variable part
} bitset_t;


int lnew(lua_State *L);

int lnew_from_string(lua_State *L);

int lnew_from_tuple(lua_State *L);


using Op = void(uint8_t *dst, const uint8_t *src, uint64_t len);

template<Op>
int lbitwise_in_place(lua_State *L);

template<Op>
int lbitwise_tuple_in_place(lua_State *L);

template<Op>
int lbitwise_uint_keys(lua_State *L);


int lto_tuple(lua_State *L);

int lto_string(lua_State *L);


static const char libname[] = "bitset";

static const struct luaL_Reg bitset_f[] = {
    {"new",             lnew},
    {"new_from_string", lnew_from_string},
    {"new_from_tuple",  lnew_from_tuple},

    {"bor_uint_keys",   lbitwise_uint_keys<bit_or>},
    {"band_uint_keys",  lbitwise_uint_keys<bit_and>},
    {"bxor_uint_keys",  lbitwise_uint_keys<bit_xor>},

    {nullptr,           nullptr},
};

static const struct luaL_Reg bitset_m[] = {
    {"bor_in_place",        lbitwise_in_place<bit_or>},
    {"band_in_place",       lbitwise_in_place<bit_and>},
    {"bxor_in_place",       lbitwise_in_place<bit_xor>},

    {"bor_tuple_in_place",  lbitwise_tuple_in_place<bit_or>},
    {"band_tuple_in_place", lbitwise_tuple_in_place<bit_and>},
    {"bxor_tuple_in_place", lbitwise_tuple_in_place<bit_xor>},

    {"to_tuple",            lto_tuple},
    {"to_string",           lto_string},
    {"__tostring",          lto_string},

    {nullptr,               nullptr},
};

int luaopen_bitset(lua_State *L) {
    luaL_newmetatable(L, libname);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, bitset_m, 0);
    luaL_register(L, libname, bitset_f);

    return 1;
}

#endif //BITSET_BITSET_H
