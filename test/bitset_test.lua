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

    bs1:bor_in_place(bs2)

    t.assert_equals(bs1:to_string(), res:to_string())
end

g.test_bor_tuple_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')

    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    box.space.test_to_tuple:insert { 2, bs2:to_tuple() }
    local bs2_tuple = box.space.test_to_tuple:get(2)

    local res = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\xFF\x00')

    bs1:bor_tuple_in_place(bs2_tuple, 2)
    t.assert_equals(bs1:to_string(), res:to_string())
end

g.test_new_from_tuple = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    box.space.test_to_tuple:insert { 1, bs1:to_tuple() }
    local bs1_tuple = box.space.test_to_tuple:get(1)
    local bs2 = bitset.new_from_tuple(bs1_tuple, 2)

    t.assert_equals(bs1:to_string(), bs2:to_string())
end

g.test_bor_uint_keys = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    box.space.test_to_tuple:insert { 3, bs1:to_tuple() }

    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    box.space.test_to_tuple:insert { 4, bs2:to_tuple() }

    local expected = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\xFF\x00')

    local result = bitset.bor_uint_keys(box.space.test_to_tuple.id, 0, 2, { 3, 4 })
    t.assert_equals(result:to_string(), expected:to_string())
end
