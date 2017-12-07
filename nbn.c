/*
 * not-so-bignum library
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "nbn.h"

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
int BN_top_set(BN *bn) {
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
    if (res == a) {
        /* abort(); */
        return;
    }
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
    BN *BN_##name(void) {                                               \
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

void BN_shr_raw(uint8_t *n, unsigned int len, uint64_t shift) {
    int i = 0;
    uint8_t offset = shift % 8;
    uint8_t bytes  = shift / 8;
    uint8_t mask   = 0;

    for (i = 0; i < offset; i++)
        mask |= 1 << i;

    /* move around the bits */
    if (offset) {
        i = len - 1;
        do {
            uint8_t carry = n[i] & mask;
            n[i] >>= offset;
            if (carry && i - 1 >= 0) {
                n[i - 1] |= carry << (8 - offset);
            }
            i--;
        } while (i > 0);
    }

    /* move around the bytes */
    if (bytes) {
        i = 0;
        while (i < len) {
            n[i] = n[i + 1];
            n[i + 1] = 0;
            i++;
        }
    }

}

void BN_shr_u8(BN *result, BN *a, uint8_t shr) {
    BN_copy(result, a);
    BN_fix(result);
    BN_shr_raw(result->n, result->top + 1, shr);
    BN_fix(result);
}

void BN_shr_u64(BN *result, BN *a, uint64_t shr) {
    BN_copy(result, a);
    BN_fix(result);
    BN_shr_raw(result->n, result->top + 1, shr);
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
    uint16_t carry = 0;

    BN_copy(result, a);
    BN_expand(result, result->top + 2);
    BN_unfix(result);

    carry = ((uint16_t)a->n[0] + i) > 255;

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
/* shift and add method */
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

/* division: q, r = a / b */
/* repeated division method */
void BN_div(BN *q, BN *r, BN *n, BN *d) {
    BN *tmp1 = BN_new();
    BN *tmp2 = BN_new();
    BN_copy(tmp1, n);
    BN_copy(tmp2, BN_zero());

    while (BN_cmp(tmp1, d) >= 0) {
        BN_sub(r, tmp1, d);
        BN_copy(tmp1, r);

        BN_add_u8(q, tmp2, 1);
        BN_copy(tmp2, q);
    }

    BN_copy(r, tmp1);
    BN_copy(q, tmp2);

    BN_free(tmp1);
    BN_free(tmp2);

    BN_fix(q);
    BN_fix(r);
}

/* print bignum as hex to screen */
void BN_print(BN *bn) {
    int i = bn->top;
    do {
        printf("%02x", bn->n[i]);
    } while (i-- > 0);
    printf("\n");
}

static uint8_t atoh(char c) {
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
    uint8_t i = 0;
    uint8_t size = (len > 1) ? len / 2 : 1;
    int16_t d;

    BN_expand(bn, size + 1);
    BN_unfix(bn);

    bn->top = size - 1;
    d = bn->top;

    if ((len % 2) != 0) {
        bn->n[d--] = atoh(str[i]);
        ++i;
    }

    for (; i < len; i += 2) {
        assert(d >= 0);
        bn->n[d--] = (atoh(str[i]) << 4) | atoh(str[i+1]);
    }

    BN_fix(bn);

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
