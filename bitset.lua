local ffi = require('ffi')
local bitset_internal = require('bitset.internal')

local bitset = {}
bitset.__index = bitset

function bitset.new(size, value)
    local bitset_cdata = ffi.new('uint8_t[?]', size)
    ffi.fill(bitset_cdata, size, value)

    return setmetatable({
        _cdata = bitset_cdata,
        _size = size,
    }, bitset)
end

function bitset.new_from_string(string)
    local size = #string
    local bitset_cdata = ffi.new('uint8_t[?]', size)
    ffi.copy(bitset_cdata, string, size)

    return setmetatable({
        _cdata = bitset_cdata,
        _size = size,
    }, bitset)
end

function bitset:bor_in_place(other)
    bitset_internal.bor_in_place(self._cdata, other._cdata, self._size)
end

function bitset:bor_tuple_in_place(tuple, field_no)
    bitset_internal.bor_tuple_in_place(self._cdata, self._size, tuple, field_no - 1)
end

function bitset:tostring()
    return ffi.string(self._cdata, self._size)
end

function bitset:to_tuple()
    local tuple = bitset_internal.to_tuple(self._cdata, self._size)
    box.internal.tuple.bless(tuple)
    return tuple
end

return bitset
