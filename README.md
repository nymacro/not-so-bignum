# not-so-bignum

Simple (inefficient) bignum implementation in C.

## Why?

To better understand how arbitrary precision maths works on
computers, in a way which is free from special tricks and
optimisations which reduce readability.

The algorithms used for the various operators here are
extremely inefficient purposely. This is not meant to be
used for any serious purpose.

## Available Functionality

Most numeric operations require a result to be specified.
This result must not be the same as the other operands.

Construction/Destruction:
* New bignum via `BN_new`
* New bignum from hexadecimal string via `BN_new_from_hex`
* Free bignum via `BN_free`

Integer operations:
* Addition via `BN_add`
* Subtraction via `BN_sub`
* Multiplication via `BN_mul`
* Division with remainder via `BN_div`
* Increment by one via `BN_inc`
* Decrement by one via `BN_dec`
* Bitwise left shift via `BN_shl`
* Bitwise right shift via `BN_shr`

Conversion functions:
* From hexadecimal via `BN_from_hex`
* To string via `BN_to_string`

Other functionality:
* Comparison via `BN_cmp`
* Maximum via `BN_max`
* Minimum via `BN_min`

## Building

```
make
```

## Testing

```
# run simple C test
./test

# run randomized Ruby test
cd ruby
bundle install
bundle exec rake spec
```
