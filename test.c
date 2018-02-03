/*
 * not-so-bignum library
 */

#include <stdio.h>
#include <string.h>
#include "nbn.h"

#define BN_assert_eq(actual, expected)          \
    {                                           \
        BN *e__ = BN_new_from_hex(expected);    \
        assert(BN_cmp(actual, e__) == 0);       \
        BN_free(e__);                           \
    }

int main(int argc, char *argv[]) {
    BN *a = BN_new_from_hex("feff");
    for (unsigned int i = 0; i < 17; i++)
        BN_dec(a);
    BN_print(a); /* => feee */
    BN_assert_eq(a, "feee");

    BN *b = BN_new_from_hex("0f10");
    for (unsigned int i = 0; i < 20000; i++)
        BN_inc(b);
    BN_print(b); /* => 5d30 */
    BN_assert_eq(b, "5d30");

    BN *z = BN_new_from_hex("ffff");
    BN_inc(z);
    BN_print(z); /* => 010000 */
    BN_assert_eq(z, "010000");
    BN_free(z);

    puts("\nComparison");
    puts("----------");
    printf("a <=> b :: %i\n", BN_cmp(a, b));
    printf("b <=> a :: %i\n", BN_cmp(b, a));
    printf("a <=> 0 :: %i\n", BN_cmp(a, BN_zero()));
    printf("b <=> 0 :: %i\n", BN_cmp(b, BN_zero()));
    printf("0 <=> 0 :: %i\n", BN_cmp(BN_zero(), BN_zero()));

    puts("\nAddition");
    puts("--------");
    BN *c = BN_new();
    printf("0xfeee + 0x5d30 = ");
    BN_add(c, a, b);
    BN_print(c); /* => 015c1e */
    BN_assert_eq(c, "015c1e");
    BN_free(c);

    puts("\nSubtraction");
    puts("-----------");
    BN *d = BN_new();
    printf("0xfeee + 0x5d30 = ");
    BN_sub(d, a, b);
    BN_print(d); /* => a1be */
    BN_assert_eq(d, "a1be");
    BN_free(d);

    puts("\nFrom Hex");
    puts("--------");
    BN *e = BN_new_from_hex("f");
    BN_print(e); /* => 0f */
    BN_assert_eq(e, "0f");
    BN_free(e);

    puts("\nAdd/Sub U8");
    puts("------");
    BN *x = BN_new_from_hex("ff");
    BN *r = BN_new();
    printf("255 + 32 = ");
    BN_add_u8(r, x, 32);
    BN_print(r); /* => 011f */
    BN_assert_eq(r, "011f");
    printf("287 - 32 = ");
    BN_sub_u8(x, r, 32);
    BN_print(x); /* => ff */
    BN_assert_eq(x, "ff");
    printf("255 + 255 = ");
    BN_add(r, x, x);
    BN_print(r);
    BN_assert_eq(r, "01fe");

    printf("5 - 3 = ");
    BN_from_hex(x, "05");
    BN_sub_u8(r, x, 3);
    BN_print(r); /* => 02 */
    BN_assert_eq(r, "02");
    BN_free(x);

    puts("\nShift");
    puts("-----");
    printf("1 << 2 = ");
    BN_shl_u8(r, BN_one(), 2);
    BN_print(r);
    printf("2 << 3 = ");
    BN_shl_u8(r, BN_two(), 3);
    BN_print(r);
    printf("1 << 16 = ");
    BN_shl_u8(r, BN_one(), 16);
    BN_print(r);
    printf("1 << 15 = ");
    BN_shl_u8(r, BN_one(), 15);
    BN_print(r);
    printf("255 << 11 = ");
    BN *ff = BN_new_from_hex("ff");
    BN_shl_u8(r, ff, 11); /* => 07f800 */
    BN_print(r);
    BN_assert_eq(r, "07f800");

    printf("255 << 1 = ");
    BN_shl_u8(r, ff, 1);
    BN_print(r); /* => 01fe */
    BN_assert_eq(r, "01fe");
    printf("255 << 2 = ");
    BN_shl_u8(r, ff, 2);
    BN_print(r); /* => 03fc */
    BN_assert_eq(r, "03fc");
    printf("255 << 3 = ");
    BN_shl_u8(r, ff, 3);
    BN_print(r); /* => 07f8 */
    BN_assert_eq(r, "07f8");
    printf("255 << 4 = ");
    BN_shl_u8(r, ff, 4);
    BN_print(r); /* => 0ff0 */
    BN_assert_eq(r, "0ff0");
    printf("255 << 5 = ");
    BN_shl_u8(r, ff, 5);
    BN_print(r); /* => 1fe0 */
    BN_assert_eq(r, "1fe0");
    printf("255 << 6 = ");
    BN_shl_u8(r, ff, 6);
    BN_print(r); /* => 3fc0 */
    BN_assert_eq(r, "3fc0");
    printf("255 << 7 = ");
    BN_shl_u8(r, ff, 7);
    BN_print(r); /* => 7f80 */
    BN_assert_eq(r, "7f80");
    printf("255 << 8 = ");
    BN_shl_u8(r, ff, 8);
    BN_print(r); /* => ff00 */
    BN_assert_eq(r, "ff00");

    printf("1 << 1024 = ");
    BN_shl_u64(r, BN_one(), 1024);
    BN_print(r);

    printf("255 >> 1 = ");
    BN_shr_u8(r, ff, 1);
    BN_print(r); /* => 7f */
    BN_assert_eq(r, "7f");

    printf("255 >> 4 = ");
    BN_shr_u8(r, ff, 4);
    BN_print(r); /* => 0f */
    BN_assert_eq(r, "0f");

    printf("255 >> 7 = ");
    BN_shr_u8(r, ff, 7);
    BN_print(r); /* => 01 */
    BN_assert_eq(r, "01");

    printf("255 >> 8 = ");
    BN_shr_u8(r, ff, 8);
    BN_print(r); /* => 00 */
    BN_assert_eq(r, "00");

    BN_from_hex(ff, "ff00");
    printf("0xff00 >> 8 = ");
    BN_shr_u8(r, ff, 8);
    BN_print(r); /* => ff */
    BN_assert_eq(r, "ff");

    BN_from_hex(ff, "0fff");
    printf("4095 >> 4 = ");
    BN_shr_u8(r, ff, 4);
    BN_print(r); /* => ff */
    BN_assert_eq(r, "ff");

    printf("1 << 10 >> 10 = ");
    BN_shl_u8(ff, BN_one(), 10);
    BN_shr_u8(r, ff, 10);
    BN_print(r);
    BN_assert_eq(r, "01");

    BN_free(ff);

    puts("\nMultiplication");
    puts("--------------");

    printf("3 * 3 = ");
    BN_mul(r, BN_three(), BN_three());
    BN_print(r);
    BN_assert_eq(r, "09");

    printf("2 * 1 = ");
    BN_mul(r, BN_two(), BN_one());
    BN_print(r);
    BN_assert_eq(r, "02");

    printf("8 * 8 = ");
    BN_mul(r, BN_eight(), BN_eight());
    BN_print(r);
    BN_assert_eq(r, "40");

    printf("2 * 2 = ");
    BN_mul(r, BN_two(), BN_two());
    BN_print(r);
    BN_assert_eq(r, "04");

    printf("2 * 4 = ");
    BN_mul(r, BN_two(), BN_four());
    BN_print(r);
    BN_assert_eq(r, "08");

    printf("1 * 1 = ");
    BN_mul(r, BN_one(), BN_one());
    BN_print(r);
    BN_assert_eq(r, "01");

    printf("8 * 0 = ");
    BN_mul(r, BN_eight(), BN_zero());
    BN_print(r);
    BN_assert_eq(r, "00");

    printf("255 * 255 = ");
    BN_from_hex(a, "ff");
    BN_from_hex(b, "ff");
    BN_mul(r, a, b);
    BN_print(r); /* => fe01 */
    BN_assert_eq(r, "fe01");

    printf("0xffffff * 2 = ");
    BN_from_hex(a, "ffffff");
    BN_from_hex(b, "02");
    BN_mul(r, a, b);
    BN_print(r); /* => 01fffffe */
    BN_assert_eq(r, "01fffffe");

    printf("0xdeadbeef * 0xf0000000 = ");
    BN_from_hex(a, "deadbeef");
    BN_from_hex(b, "f0000000");
    BN_mul(r, a, b);
    BN_print(r); /* => d0c2e30010000000 */
    BN_assert_eq(r, "d0c2e30010000000");

    puts("\nDivision");
    puts("--------------");

    printf("1 / 1 = ");
    BN_div(a, b, BN_one(), BN_one());
    BN_print(a);
    BN_assert_eq(a, "01");
    BN_assert_eq(b, "00");

    printf("2 / 1 = ");
    BN_div(a, b, BN_two(), BN_one());
    BN_print(a);
    BN_assert_eq(a, "02");
    BN_assert_eq(b, "00");

    printf("1 / 2 = ");
    BN_div(a, b, BN_one(), BN_two());
    BN_print(a);
    BN_assert_eq(a, "00");
    BN_assert_eq(b, "01");

    printf("10 / 3 = ");
    BN_div(a, b, BN_ten(), BN_three());
    BN_print(a);
    BN_assert_eq(a, "03");
    BN_assert_eq(b, "01");

    BN_from_hex(a, "ffffffff");
    BN_from_hex(b, "3dfe");
    BN *r2 = BN_new();
    BN_div(r, r2, a, b);
    BN_assert_eq(r, "4212a");

    puts("\nConversion");
    puts("--------------");

    char buf[20];
    BN_from_hex(r, "abcd");
    int i = BN_to_string(r, buf, sizeof(buf));
    buf[i] = 0;
    assert(strcmp(buf, "abcd") == 0);

    BN_free(a);
    BN_free(b);
    BN_free(r);

    return 0;
}
