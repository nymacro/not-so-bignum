/*
 * not-so-bignum library
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
    unsigned int top;
    unsigned int len;
    uint8_t *n;
} BN;

BN *BN_new() {
    const unsigned int size = 2;
    BN *bn;
    bn = malloc(sizeof(BN));
    if (bn == NULL)
        return NULL;
    bn->len = size;
    bn->top = 0;
    bn->n = malloc(sizeof(uint8_t) * size);
    if (bn->n == NULL) {
        free(bn);
        return NULL;
    }

    return bn;
}

void BN_free(BN *bn) {
    free(bn->n);
    free(bn);
}

/* adjust the top down to the element with a non-zero value */
void BN_fix(BN *bn) {
    while (bn->top > 0) {
        if (bn->n[bn->top] != 0)
            break;
        bn->top--;
    }
}

/* expand top to len */
void BN_unfix(BN *bn) {
    while (bn->top < bn->len - 1) {
        bn->n[++bn->top] = 0;
    }
}

/* allocate additional memory for the number, does not modify "top" */
int BN_expand(BN *bn, unsigned int size) {
    if (bn->len >= size)
        return 1;
    bn->n = realloc(bn->n, sizeof(uint8_t) * size);
    if (bn->n == NULL)
        abort();
    bn->len = size;
    for (unsigned int i = bn->top + 1; i < bn->len; i++)
        bn->n[i] = 0;
    return 1;
}

/* is the MSB set on top? */
int BN_top_set(BN *bn)
{
    return bn->n[bn->top] & 0x80;
}

void BN_dec_raw(uint8_t *n, unsigned int len) {
    unsigned int i = 0;
    while (i < len) {
        if (n[i] > 0) {
            --n[i];
            break;
        } else {
            n[i] = ~n[i];
        }
        i++;
    }
}

/* decrement */
void BN_dec(BN *bn) {
    BN_dec_raw(bn->n, bn->top + 1);
    BN_fix(bn);
}

/* increment raw buf */
void BN_inc_raw(uint8_t *n, unsigned int len)
{
    unsigned int i = 0;
    while (i < len) {
        if (n[i] < 255) {
            ++n[i];
            break;
        } else {
            n[i] = ~n[i];
        }
        i++;
    }
}

/* increment */
void BN_inc(BN *bn) {
    BN_expand(bn, bn->top + 2);
    BN_unfix(bn);
    BN_inc_raw(bn->n, bn->top + 1);
    BN_fix(bn);
}

/* comparison */
int BN_cmp(BN *a, BN *b) {
    unsigned int i;
    BN_fix(a);
    BN_fix(b);
    if (a->top > b->top)
        return 1;
    if (b->top > a->top)
        return -1;
    i = a->top;
    do {
        if (a->n[i] > b->n[i])
            return 1;
        if (b->n[i] > a->n[i])
            return -1;
    } while (i-- > 0);
    return 0;
}

/* number copy (copies to existing BN) */
void BN_copy(BN *res, BN *a) {
    unsigned int i;
    if (res == a)
        abort();
    if (res->len < a->len) {
        res->n = realloc(res->n, sizeof(uint8_t) * a->len);
        res->len = a->len;
    }
    res->top = a->top;
    for (i = 0; i <= a->top; i++) {
        res->n[i] = a->n[i];
    }
    for (++i; i < a->len; i++) {
        res->n[i] = 0;
    }
}

#define BN_const_define(name, value)                                    \
    BN *BN_##name() {                                                   \
        static const uint8_t value_[] = value;                          \
        static BN name = { sizeof(value_)-1, sizeof(value_), NULL };    \
        name.n = (uint8_t*)value_;                                      \
        return &name;                                                   \
    }

BN_const_define(zero, { 0x00 });
BN_const_define(one, { 0x01 });
BN_const_define(two, { 0x02 });
BN_const_define(three, { 0x03 });
BN_const_define(four, { 0x04 });
BN_const_define(five, { 0x05 });
BN_const_define(six, { 0x06 });
BN_const_define(seven, { 0x07 });
BN_const_define(eight, { 0x08 });
BN_const_define(nine, { 0x09 });
BN_const_define(ten, { 0x0a });

BN *BN_max(BN *a, BN *b) {
    int cmp = BN_cmp(a, b);
    if (cmp < 0)
        return a;
    else if (cmp > 1)
        return b;
    return a;
}

BN *BN_min(BN *a, BN *b) {
    int cmp = BN_cmp(b, a);
    if (cmp < 0)
        return a;
    else if (cmp > 1)
        return b;
    return a;
}

/* len should be large enough to contain the shifted amount */
void BN_shl_raw(uint8_t *n, unsigned int len, uint64_t shift) {
    int i = 0;
    uint8_t offset = shift % 8;
    uint8_t bytes  = shift / 8;
    uint8_t mask   = 0;

    for (i = 7; i > 7 - offset; i--)
        mask |= 1 << i;

    /* move around the bytes */
    if (bytes) {
        i = len - 1;
        do {
            n[i] = n[i - bytes];
            i--;
        } while (i >= bytes);

        while (i >= 0)
            n[i--] = 0;
    }

    /* move around the bits */
    if (offset) {
        i = len - 1;
        while (i >= 0) {
            uint8_t carry = n[i] & mask;
            n[i] <<= offset;
            if (carry) {
                n[i + 1] |= carry >> (8 - offset);
            }
            i--;
        }
    }
}

void BN_shl_u8(BN *result, BN *a, uint8_t shl) {
    BN_copy(result, a);
    BN_expand(result, result->top + 3 + (shl / 8));
    BN_unfix(result);
    BN_shl_raw(result->n, result->len, shl);
    BN_fix(result);
}

void BN_shl_u64(BN *result, BN *a, uint64_t shl) {
    BN_copy(result, a);
    BN_expand(result, result->top + 3 + (shl / 8));
    BN_unfix(result);
    BN_shl_raw(result->n, result->len, shl);
    BN_fix(result);
}

/* addition: r = a + b */
void BN_add(BN *result, BN *a, BN *b) {
    unsigned int max_top = (a->top > b->top) ? a->top : b->top;
    BN_copy(result, a);
    BN_expand(result, max_top + 2);
    BN_unfix(result);

    unsigned int i = 0;
    while (i <= b->top) {
        uint16_t carry = ((uint16_t)result->n[i] + b->n[i]) > 255;

        result->n[i] += b->n[i];
        if (carry) {
            BN_inc_raw(result->n + i + 1, result->top - i);
        }

        i++;
    }

    BN_fix(result);
}

void BN_add_u8(BN *result, BN *a, uint8_t i) {
    uint8_t carry = 0;

    BN_copy(result, a);
    BN_expand(result, result->top + 2);
    BN_unfix(result);

    if (i & 0x80 || a->n[0] & 0x80) {
        carry = 1;
    }

    result->n[0] += i;
    if (carry) {
        BN_inc_raw(result->n + 1, result->top);
    }

    BN_fix(result);
}

/* subtraction: r = a - b */
void BN_sub(BN *result, BN *a, BN *b) {
    BN_copy(result, a);
    BN_fix(result);
    BN_fix(b);

    int i = b->top;
    while (i >= 0) {
        uint8_t borrow = b->n[i] > result->n[i];

        result->n[i] -= b->n[i];
        if (borrow) {
            BN_dec_raw(result->n + i + 1, result->top - i);
        }

        i--;
    }

    BN_fix(result);
}

void BN_sub_u8(BN *result, BN *a, uint8_t i) {
    BN_copy(result, a);
    BN_fix(result);

    uint8_t borrow = 0;

    if (i > a->n[0]) {
        borrow = 1;
    }

    if (borrow) {
        result->n[0] -= i;
        BN_dec_raw(result->n + 1, result->top);
    } else {
        result->n[0] -= i;
    }

    BN_fix(result);
}

/* multiplication: r = a * b */
void BN_mul(BN *result, BN *a, BN *b) {
    uint8_t max_top = (a->top > b->top) ? a->top : b->top;
    if (BN_top_set(a) || BN_top_set(b)) {
        max_top += 1;
    }
    BN_copy(result, BN_zero());
    BN_expand(result, (max_top + 2) * 2);

    BN *tmp1 = BN_new();
    BN *tmp2 = BN_new();

    unsigned int i = 0;
    while (i <= b->top) {
        for (int bit = 0; bit < 8; bit++) {
            uint8_t z = b->n[i] & (1 << bit);
            if (z) {
                BN_shl_u8(tmp1, a, i * 8 + bit);
                BN_copy(tmp2, result);
                BN_add(result, tmp1, tmp2);
            }
        }
        i++;
    }
    BN_free(tmp1);
    BN_free(tmp2);
}

/* division: r = a / b */
void BN_div(BN *result, BN *a, BN *b) {
    /* DO ME */
}

/* print bignum as hex to screen */
void BN_print(BN *bn) {
    unsigned int i = bn->top;
    do {
        printf("%02x", bn->n[i]);
    } while (i-- > 0);
    printf("\n");
}

uint8_t atoh(char c) {
    switch (tolower(c)) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
    }
    return 0;
}

/* make a bignum from a hex string */
int BN_from_hex(BN *bn, char *str) {
    size_t len = strlen(str);
    unsigned int i = 0;
    unsigned int d;
    unsigned int size = (len > 1) ? len / 2 : 1;

    BN_expand(bn, size);

    bn->top = size - 1;
    d = bn->top;

    if ((len % 2) != 0) {
        bn->n[d--] = atoh(str[i]);
        ++i;
    }

    for (; i < len; i += 2) {
        bn->n[d--] = (atoh(str[i]) << 4) | atoh(str[i+1]);
    }

    return 1;
}

BN *BN_new_from_hex(char *str) {
    BN *bn = BN_new();
    if (!bn)
        return NULL;
    if (!BN_from_hex(bn, str)) {
        BN_free(bn);
        return NULL;
    }
    return bn;
}

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

    printf("a <=> b :: %i\n", BN_cmp(a, b));
    printf("b <=> a :: %i\n", BN_cmp(b, a));
    printf("a <=> 0 :: %i\n", BN_cmp(a, BN_zero()));
    printf("b <=> 0 :: %i\n", BN_cmp(b, BN_zero()));
    printf("0 <=> 0 :: %i\n", BN_cmp(BN_zero(), BN_zero()));

    puts("\nAddition");
    puts("--------");
    BN *c = BN_new();
    BN_add(c, a, b);
    BN_print(c); /* => 015c1e */
    BN_assert_eq(c, "015c1e");
    BN_free(c);

    puts("\nSubtraction");
    puts("-----------");
    BN *d = BN_new();
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
    BN_add_u8(r, x, 32);
    BN_print(r); /* => 011f */
    BN_assert_eq(r, "011f");
    BN_sub_u8(x, r, 32);
    BN_print(x); /* => ff */
    BN_assert_eq(x, "ff");
    BN_free(x);

    x = BN_new_from_hex("05");
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

    BN_free(a);
    BN_free(b);
    BN_free(r);

    return 0;
}
