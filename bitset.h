#ifndef BITSET_BITSET_H
#define BITSET_BITSET_H

//#include <lua.h>
#include <lauxlib.h>
//#include <lualib.h>

int lmerge(struct lua_State *L);

int lmerge_tuple(struct lua_State *L);

int lmerge_tuples(struct lua_State *L);

static const struct luaL_Reg functions[] = {
    {"merge",        lmerge},
    {"merge_tuple",  lmerge_tuple},
    {"merge_tuples", lmerge_tuples},
    {NULL, NULL}
};

int luaopen_hello(lua_State *L) {
    luaL_register(L, "bitset", functions);
    return 1;
}

#endif //BITSET_BITSET_H
