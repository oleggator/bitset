local t = require('luatest')
local g = t.group('bitset')

local bitset = require('bitset')

g.before_all(function()
    box.cfg {}

    box.schema.create_space('test_to_tuple')
    box.space.test_to_tuple:create_index('pk', { parts = { { 1, 'integer' } } })
end)

g.after_all(function()
    box.space.test_to_tuple:drop()
end)

g.test_bor_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    local res = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\xFF\x00')

    bitset.bor_in_place(bs1, bs2)

    t.assert_equals(bitset.to_string(bs1), bitset.to_string(res))
end

g.test_bor_tuple_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')

    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    box.space.test_to_tuple:insert { 2, bitset.to_tuple(bs2) }
    local bs2_tuple = box.space.test_to_tuple:get(2)

    local res = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\xFF\x00')

    bitset.bor_tuple_in_place(bs1, bs2_tuple, 2)
    t.assert_equals(bitset.to_string(bs1), bitset.to_string(res))
end

g.test_new_from_tuple = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    box.space.test_to_tuple:insert { 1, bitset.to_tuple(bs1) }
    local bs1_tuple = box.space.test_to_tuple:get(1)
    local bs2 = bitset.new_from_tuple(bs1_tuple, 2)

    t.assert_equals(bitset.to_string(bs1), bitset.to_string(bs2))
end
