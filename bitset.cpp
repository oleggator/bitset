#include <tarantool/module.h>
#include <cinttypes>
#include "msgpuck/msgpuck.h"

#include "bitset.h"

int luaopen_bitset(lua_State *L) {
    luaL_newmetatable(L, libname);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, bitset_m, 0);
    luaL_register(L, libname, bitset_f);

    return 1;
}


inline bitset_t *check_bitset(lua_State *L, int idx) {
    return static_cast<bitset_t *>(luaL_checkudata(L, idx, libname));
}


size_t get_header_len(uint64_t len) {
    if (len <= UINT8_MAX) {
        return 2;
    }
    if (len <= UINT16_MAX) {
        return 3;
    }
    return 5;
}


int lnew(lua_State *L) {
    uint64_t len = luaL_checkuint64(L, 1);

    size_t bin_header_size = get_header_len(len);
    size_t udata_size = sizeof(bitset_t) + (bin_header_size + len - 1) * sizeof(uint8_t);
    bitset_t *bitset = (bitset_t *) lua_newuserdata(L, udata_size);

    bitset->size = len;
    bitset->bin_header_size = bin_header_size;

    char *bitset_body = mp_encode_binl((char *) bitset->msgpack, len);
    memset(bitset_body, 0, len * sizeof(uint8_t));

    luaL_getmetatable(L, libname);
    lua_setmetatable(L, -2);

    return 1;
}


int lnew_from_string(lua_State *L) {
    size_t len;
    const char *str = lua_tolstring(L, 1, &len);

    size_t bin_header_size = get_header_len(len);
    size_t udata_size = sizeof(bitset_t) + (bin_header_size + len - 1) * sizeof(uint8_t);
    bitset_t *bitset = (bitset_t *) lua_newuserdata(L, udata_size);

    bitset->size = len;
    bitset->bin_header_size = bin_header_size;

    char *bitset_body = mp_encode_binl((char *) bitset->msgpack, len);
    memcpy(bitset_body, str, len);

    luaL_getmetatable(L, libname);
    lua_setmetatable(L, -2);

    return 1;
}


bitset_t *new_from_tuple(lua_State *L, box_tuple_t *tuple, uint64_t field_no) {
    const char *src_msgpack = box_tuple_field(tuple, field_no - 1);
    if (src_msgpack == nullptr) {
        return nullptr;
    }
    const char *src_msgpack_cursor = src_msgpack;
    // TODO handle wrong type
    uint64_t len = mp_decode_binl(&src_msgpack_cursor);
    uint64_t bin_header_size = src_msgpack_cursor - src_msgpack;

    size_t udata_size = sizeof(bitset_t) + (bin_header_size + len - 1) * sizeof(uint8_t);
    bitset_t *bitset = (bitset_t *) lua_newuserdata(L, udata_size);

    bitset->size = len;
    bitset->bin_header_size = bin_header_size;
    memcpy(bitset->msgpack, src_msgpack, bin_header_size + len);

    luaL_getmetatable(L, libname);
    lua_setmetatable(L, -2);

    return bitset;
}


int lnew_from_tuple(lua_State *L) {
    box_tuple_t *tuple = luaT_istuple(L, 1);
    if (tuple == nullptr) {
        return luaL_error(L, "Usage bitset.new_from_tuple(tuple, field_no)");
    }
    uint64_t field_no = luaL_checkuint64(L, 2);

    box_tuple_ref(tuple);
    new_from_tuple(L, tuple, field_no);
    box_tuple_unref(tuple);

    return 1;
}


template<Op op>
int lbitwise_in_place(lua_State *L) {
    bitset_t *dst_bitset = check_bitset(L, 1);
    bitset_t *src_bitset = check_bitset(L, 2);

    op(dst_bitset->msgpack + dst_bitset->bin_header_size,
       src_bitset->msgpack + src_bitset->bin_header_size,
       src_bitset->size);

    return 0;
}


template<Op op>
int bitwise_tuple_in_place(bitset_t *dst_bitset, box_tuple_t *src_tuple, uint64_t field_no) {
    const auto *src = (const uint8_t *) box_tuple_field(src_tuple, field_no - 1);
    if (src == nullptr) {
        return 1;
    }

    // TODO handle wrong type
    mp_decode_binl((const char **) &src); // decode binary length

    op(dst_bitset->msgpack + dst_bitset->bin_header_size, src, dst_bitset->size);
    return 0;
}

template<Op op>
int lbitwise_tuple_in_place(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    box_tuple_t *tuple = luaT_istuple(L, 2);
    uint64_t field_no = luaL_checkuint64(L, 3);

    box_tuple_ref(tuple);
    int err = bitwise_tuple_in_place<op>(bitset, tuple, field_no);
    box_tuple_unref(tuple);

    if (err != 0) {
        return luaL_error(L, "Wrong field index");
    }

    return 0;
}


template<Op op>
int lbitwise_uint_keys(lua_State *L) {
    const int space_id_index = 1;
    const int index_id_index = 2;
    const int field_no_index = 3;
    const int keys_table_index = 4;

    const uint64_t space_id = luaL_checkuint64(L, space_id_index);
    const uint64_t index_id = luaL_checkuint64(L, index_id_index);
    const uint64_t field_no = luaL_checkuint64(L, field_no_index);
    if (!lua_istable(L, keys_table_index)) {
        return luaL_error(L,
                          "Usage bitset.bor_uint_keys(space_id, index_id, field_no, keys_table)");
    }

    size_t keys_table_len = lua_objlen(L, keys_table_index);
    if (keys_table_len < 1) {
        return luaL_error(L,
                          "Usage bitset.bor_uint_keys(space_id, index_id, field_no, keys_table)");
    }

    lua_rawgeti(L, keys_table_index, 1);
    uint64_t id = luaL_checkuint64(L, -1);
    lua_pop(L, 1);

    const int key_msgpack_len = 10;  // max uint64 msgpack size + array header
    char key_msgpack[key_msgpack_len];
    char *key_part = mp_encode_array(key_msgpack, 1);
    mp_encode_uint(key_part, id);

    box_tuple_t *tuple;
    int err = box_index_get(space_id, index_id,
                            key_msgpack, key_msgpack + key_msgpack_len,
                            &tuple);
    if (err != 0) {
        return luaT_error(L);
    }
    if (tuple == nullptr) {
        return luaL_error(L, "Tuple with key %" PRIu64 " not found", id);
    }

    box_tuple_ref(tuple);
    bitset_t *bitset = new_from_tuple(L, tuple, field_no); // pushes bitset to lua stack
    box_tuple_unref(tuple);

    for (size_t i = 2; i <= keys_table_len; ++i) {
        lua_rawgeti(L, keys_table_index, i);
        id = luaL_checkuint64(L, -1);
        lua_pop(L, 1);
        mp_encode_uint(key_part, id);

        err = box_index_get(space_id, index_id,
                            key_msgpack, key_msgpack + key_msgpack_len,
                            &tuple);
        if (err != 0) {
            return luaT_error(L);
        }
        if (tuple == nullptr) {
            return luaL_error(L, "Tuple with key %" PRId64 " not found", id);
        }

        box_tuple_ref(tuple);
        bitwise_tuple_in_place<op>(bitset, tuple, field_no);
        box_tuple_unref(tuple);
    }

    return 1;
}


box_tuple_t *to_tuple(const bitset_t *bitset) {
    const uint8_t *tuple_body_begin = bitset->msgpack;
    const uint8_t *tuple_body_end = bitset->msgpack + bitset->bin_header_size + bitset->size;
    return box_tuple_new(box_tuple_format_default(),
                         (const char *) tuple_body_begin,
                         (const char *) tuple_body_end);
}

int lto_tuple(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    box_tuple_t *tuple = to_tuple(bitset);
    luaT_pushtuple(L, tuple);
    return 1;
}


int lto_string(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    lua_pushlstring(L, (const char *) bitset->msgpack + bitset->bin_header_size, bitset->size);
    return 1;
}


void set_bit(bitset_t *bitset, uint64_t index, bool value) {
    uint8_t *bitset_raw = bitset->msgpack + bitset->bin_header_size;
    uint64_t byte_index = (index - 1) / 8;
    uint8_t bit_index = (index - 1) % 8;
    if (value) {
        bitset_raw[byte_index] |= 0x01 << bit_index;
    } else {
        bitset_raw[byte_index] &= ~(0x01 << bit_index);
    }
}

int lset_bit(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    uint64_t index = luaL_checkuint64(L, 2);
    if (!lua_isboolean(L, 3)) {
        return luaL_error(L, "Usage: bitset.set(u64 index, bool value)");
    }
    bool value = lua_toboolean(L, 3);

    if (index < 1 || index > bitset->size * 8) {
        return luaL_error(L, "Index must be greater than 0"
                             " and less than or equal to the size of the array");
    }
    set_bit(bitset, index, value);

    lua_pushboolean(L, value);
    return 1;
}


bool get_bit(const bitset_t *bitset, uint64_t index) {
    const uint8_t *bitset_raw = bitset->msgpack + bitset->bin_header_size;
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;
    bool bit = (bitset_raw[byte_index] >> bit_index) & 0x01;

    return bit;
}

int lget_bit(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    uint64_t index = luaL_checkuint64(L, 2);
    if (index < 1 || index > bitset->size * 8) {
        return luaL_error(L, "Index must be greater than 0"
                             " and less than or equal to the size of the array");
    }

    lua_pushboolean(L, get_bit(bitset, index - 1));
    return 1;
}


bool all(const uint8_t *bs, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        if (bs[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

int lall(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    lua_pushboolean(L, all(bitset->msgpack + bitset->bin_header_size, bitset->size));
    return 1;
}


bool any(const uint8_t *bs, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        if (bs[i] != 0x00) {
            return true;
        }
    }
    return false;
}

int lany(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    lua_pushboolean(L, any(bitset->msgpack + bitset->bin_header_size, bitset->size));
    return 1;
}


bool none(const uint8_t *bs, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        if (bs[i] != 0x00) {
            return false;
        }
    }
    return true;
}

int lnone(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    lua_pushboolean(L, none(bitset->msgpack + bitset->bin_header_size, bitset->size));
    return 1;
}


void set(uint8_t *bs, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        bs[i] = 0xFF;
    }
}

int lset(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    set(bitset->msgpack + bitset->bin_header_size, bitset->size);
    return 0;
}


void reset(uint8_t *bs, uint64_t len) {
    for (uint64_t i = 0; i < len; ++i) {
        bs[i] = 0x00;
    }
}

int lreset(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    reset(bitset->msgpack + bitset->bin_header_size, bitset->size);
    return 0;
}


uint64_t count(const uint8_t *bs, uint64_t len) {
    uint64_t counter = 0;
    for (uint64_t i = 0; i < len; ++i) {
        counter += __builtin_popcount(bs[i]);
    }
    return counter;
}

int lcount(lua_State *L) {
    bitset_t *bitset = check_bitset(L, 1);
    luaL_pushuint64(L, count(bitset->msgpack + bitset->bin_header_size, bitset->size));
    return 1;
}
