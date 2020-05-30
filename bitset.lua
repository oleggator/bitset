local ffi = require('ffi')

local bitset = {}

function bitset:new(size, value)
    local bitset_cdata = ffi.new('uint8_t[?]', size)
    if value ~= nil then
        ffi.fill(bitset_cdata, value)
    else
        ffi.fill(bitset_cdata)
    end

    return setmetatable({
        _cdata = bitset_cdata,
    }, self)
end

function bitset:band()

end

function bitset:bor()

end

function bitset:bxor()

end

function bitset:bnot()

end

function bitset:bswap()

end

function bitset:lshift()

end

function bitset:rshift()

end

function bitset:arshift()

end

return bitset
