local t = require('luatest')
local g = t.group('bitset')

local bitset = require('bitset')

g.before_all(function()
    box.cfg {
        wal_mode = 'none',
        checkpoint_interval = -1,
    }

    box.schema.create_space('space1')
    box.space.space1:create_index('pk', { parts = { { 1, 'integer' } } })

    box.schema.create_space('space2')
    box.space.space2:create_index('pk', { parts = { { 1, 'integer' } } })
    box.space.space2:create_index('secondary', { parts = { { 2, 'integer' } }, unique = false })
end)

g.after_all(function()
    box.space.space1:drop()
    box.space.space2:drop()
end)

g.test_bor_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\xFF\x00\xFF\x00\xFF\x00\xFF\x00'

    bs1:bor_in_place(bs2)
    t.assert_equals(bs1:to_string(), expected)
end

g.test_band_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\x00\x00\xFF\x00\xFF\x00\x00\x00'

    bs1:band_in_place(bs2)
    t.assert_equals(bs1:to_string(), expected)
end

g.test_bxor_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\xFF\x00\x00\x00\x00\x00\xFF\x00'

    bs1:bxor_in_place(bs2)
    t.assert_equals(bs1:to_string(), expected)
end

g.test_bor_tuple_in_place = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    local expected = '\xFF\x00\xFF\x00\xFF\x00\xFF\x00'

    box.space.space1:insert { 2, bs2:to_tuple() }
    local bs2_tuple = box.space.space1:get(2)

    bs1:bor_tuple_in_place(bs2_tuple, 2)
    t.assert_equals(bs1:to_string(), expected)
end

g.test_new_from_tuple = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    box.space.space1:insert { 1, bs1:to_tuple() }
    local bs1_tuple = box.space.space1:get(1)
    local bs2 = bitset.new_from_tuple(bs1_tuple, 2)

    t.assert_equals(bs1:to_string(), bs2:to_string())
end

g.test_bor_uint_keys = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    local expected = '\xFF\x00\xFF\x00\xFF\x00\xFF\x00'

    box.space.space1:insert { 3, bs1:to_tuple() }
    box.space.space1:insert { 4, bs2:to_tuple() }

    local result = bitset.bor_uint_keys(box.space.space1.id, 0, 2, { 3, 4 })
    t.assert_equals(result:to_string(), expected)
end

g.test_band_uint_keys = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\x00\x00\xFF\x00\xFF\x00\x00\x00'

    box.space.space1:insert { 5, bs1:to_tuple() }
    box.space.space1:insert { 6, bs2:to_tuple() }

    local result = bitset.band_uint_keys(box.space.space1.id, 0, 2, { 5, 6 })
    t.assert_equals(result:to_string(), expected)
end

g.test_bxor_uint_keys = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\xFF\x00\x00\x00\x00\x00\xFF\x00'

    box.space.space1:insert { 7, bs1:to_tuple() }
    box.space.space1:insert { 8, bs2:to_tuple() }

    local result = bitset.bxor_uint_keys(box.space.space1.id, 0, 2, { 7, 8 })
    t.assert_equals(result:to_string(), expected)
end

g.test_count = function()
    local bs = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    t.assert_equals(bs:count(), 24)
end

g.test_all = function()
    t.assert_equals(bitset.new_from_string('\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF'):all(), true)
    t.assert_equals(bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00'):all(), false)
end

g.test_none = function()
    t.assert_equals(bitset.new_from_string('\x00\x00\x00\x00\x00\x00\x00\x00'):none(), true)
    t.assert_equals(bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00'):none(), false)
end

g.test_any = function()
    t.assert_equals(bitset.new_from_string('\x00\x00\x00\x00\x00\x00\x00\x00'):any(), false)
    t.assert_equals(bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00'):any(), true)
end

g.test_set = function()
    local bs = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    bs:set()
    t.assert_equals(bs:to_string(), '\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF')
end

g.test_reset = function()
    local bs = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    bs:reset()
    t.assert_equals(bs:to_string(), '\x00\x00\x00\x00\x00\x00\x00\x00')
end

g.test_get_set_bit = function()
    local bs = bitset.new_from_string('\x00\x00\x00\x00\xFF\xFF\xFF\xFF')
    t.assert_equals(bs:get_bit(1), false)
    t.assert_equals(bs:get_bit(64), true)

    bs:set_bit(1, true)
    bs:set_bit(64, false)
    t.assert_equals(bs:get_bit(1), true)
    t.assert_equals(bs:get_bit(64), false)
end

g.test_set_bit_in_tuple_uint_key = function()
    local id = 9
    local field_no = 2
    local index_id = 0

    local bs = bitset.new_from_string('\x00\x00\x00\x00\xFF\xFF\xFF\xFF')
    box.space.space1:insert { id, bs:to_tuple() }

    local tuple = box.space.space1:get(id)
    t.assert_equals(bitset.new_from_tuple(tuple, field_no):get_bit(1), false)
    t.assert_equals(bitset.new_from_tuple(tuple, field_no):get_bit(64), true)

    bitset.set_bit_in_tuple_uint_key(box.space.space1.id, index_id, id, field_no, 1, true)
    tuple = box.space.space1:get(id)
    t.assert_equals(bitset.new_from_tuple(tuple, field_no):get_bit(1), true)

    bitset.set_bit_in_tuple_uint_key(box.space.space1.id, index_id, id, field_no, 64, false)
    tuple = box.space.space1:get(id)
    t.assert_equals(bitset.new_from_tuple(tuple, field_no):get_bit(64), false)
end

g.test_bor_uint_iter = function()
    local bs1 = bitset.new_from_string('\xFF\x00\x00\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\x00\x00\xFF\x00')
    local expected = '\xFF\x00\xFF\x00\xFF\x00\xFF\x00'

    box.space.space2:insert { 1, 1, bs1:to_tuple() }
    box.space.space2:insert { 2, 1, bs2:to_tuple() }

    local result = bitset.bor_uint_iter(box.space.space2.id, box.space.space2.index.secondary.id,
            1, 3, box.index.EQ)
    t.assert_equals(result:to_string(), expected)
end

g.test_band_uint_iter = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\x00\x00\xFF\x00\xFF\x00\x00\x00'

    box.space.space2:insert { 3, 2, bs1:to_tuple() }
    box.space.space2:insert { 4, 2, bs2:to_tuple() }

    local result = bitset.band_uint_iter(box.space.space2.id, box.space.space2.index.secondary.id,
            2, 3, box.index.EQ)
    t.assert_equals(result:to_string(), expected)
end

g.test_bxor_uint_iter = function()
    local bs1 = bitset.new_from_string('\xFF\x00\xFF\x00\xFF\x00\x00\x00')
    local bs2 = bitset.new_from_string('\x00\x00\xFF\x00\xFF\x00\xFF\x00')
    local expected = '\xFF\x00\x00\x00\x00\x00\xFF\x00'

    box.space.space2:insert { 5, 3, bs1:to_tuple() }
    box.space.space2:insert { 6, 3, bs2:to_tuple() }

    local result = bitset.bxor_uint_iter(box.space.space2.id, box.space.space2.index.secondary.id,
            3, 3, box.index.EQ)
    t.assert_equals(result:to_string(), expected)
end
