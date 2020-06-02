local t = require('luatest')
local g = t.group('bitset')

local bitset = require('bitset')

g.before_all(function()
end)

g.test_bor_in_place = function()
    local bs1 = bitset.new(8, 0)
    local bs2 = bitset.new(8, 1)

    bs1:bor_in_place(bs2)

    t.assert(bs1:tostring() == bs2:tostring())
end
