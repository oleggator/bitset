# BitSet for Tarantool
![Test bitset](https://github.com/oleggator/bitset/workflows/Test%20bitset/badge.svg)

## Requirements
- git
- make
- cmake >= 2.8
- c++11 compiler

## API
### `bitset.new(byte_count)`
Creates bitset with size in bytes

### `bitset.new_from_string(string)`
Creates bitset from string

### `bitset.new_from_tuple(tuple, field_no)`
Creates bitset from tuple binary field

### `bitset.bor_uint_keys(space_id, index_id, keys)`
Bitwise OR tuple fields by uint key

### `bitset.band_uint_keys(space_id, index_id, keys)`
Bitwise AND tuple fields by uint key

### `bitset.bxor_uint_keys(space_id, index_id, keys)`
Bitwise XOR tuple fields by uint key

### `bitset.set_bit_in_tuple_uint_key(space_id, index_id, key, field_no, bit_index)`
Sets bit in tuple field

### `bitset:bor_in_place(bitset)`
In-place bitwise OR bitset with another

### `bitset:band_in_place(bitset)`
In-place bitwise AND bitset with another

### `bitset:bxor_in_place(bitset)`
In-place bitwise XOR bitset with another

### `bitset:bor_tuple_in_place(tuple, field_no)`
In-place bitwise OR bitset with tuple binary field

### `bitset:band_tuple_in_place(tuple, field_no)`
In-place bitwise AND bitset with tuple binary field

### `bitset:bxor_tuple_in_place(tuple, field_no)`
In-place bitwise XOR bitset with tuple binary field

### `bitset:to_tuple()`
Casts bitset to Tarantool tuple

### `bitset:to_string()`
Casts bitset to string

### `bitset:get_bit(index)`
Gets bit by index

### `bitset:set_bit(index, value)`
Sets bit by index to given value

### `bitset:all()`
Checks if all bits are set to true

### `bitset:any()`
Checks if any bits are set to true

### `bitset:none()`
Checks if all bits are set to false

### `bitset:count()`
Returns the number of bits set to true

### `bitset:set()`
Sets bits to true
 
### `bitset:reset()`
Sets bits to false
