local ffi = require('ffi')
local bitset_internal = require('bitset_internal')

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

function bitset:bor_in_place(other)
    bitset_internal.bor_in_place(self._cdata, other._cdata, self._size)
end

function bitset:tostring()
    return ffi.string(self._cdata, self._size)
end

return bitset
