local t = require('luatest')
local g = t.group('bitset')

local bitset = require('bitset')

g.before_all(function()
end)

g.test_bor_in_place1 = function()
    local bs1 = bitset.new(8, 0x00)
    local bs2 = bitset.new(8, 0xFF)

    bs1:bor_in_place(bs2)

    t.assert_equals(bs1:tostring(), bs2:tostring())
end

g.test_bor_in_place2 = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    local res = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\xFF\x00')

    bs1:bor_in_place(bs2)

    t.assert_equals(bs1:tostring(), res:tostring())
end
