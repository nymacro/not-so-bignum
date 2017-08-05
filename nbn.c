#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

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
    bn->n = malloc(size);
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
    unsigned int i = bn->top;
    do {
        if (i == 0)
            break;
        if (bn->n[i] != 0)
            break;
    } while (i-- > 0);
    bn->top = i;
}

/* expand top to len */
void BN_unfix(BN *bn) {
    while (bn->top < bn->len)
        bn->n[++bn->top] = 0;
}

/* allocate additional memory for the number */
int BN_expand(BN *bn, unsigned int size) {
    uint8_t *new;
    if (bn->len >= size)
        return 1;
    new = realloc(bn->n, size);
    if (new == NULL)
        return 0;
    bn->n = new;
    bn->len = size;
    for (unsigned int i = bn->top + 1; i < bn->len; i++)
        bn->n[i] = 0;
    return 1;
}

int BN_top_set(BN *bn)
{
    return bn->n[bn->top] & 0x80;
}

void BN_expand_maybe(BN *bn)
{
    if (BN_top_set(bn)) {
        BN_expand(bn, bn->len + 1);
    }
}

void BN_dec_raw(uint8_t *n, unsigned int len) {
    unsigned int i = 0;
    do {
        if (n[i] > 0) {
            --n[i];
            break;
        } else {
            n[i] = ~n[i];
        }
    } while (i++ <= len);
}

/* decrement */
void BN_dec(BN *bn) {
    BN_dec_raw(bn->n, bn->top);
    BN_fix(bn);
}

/* increment raw buf */
void BN_inc_raw(uint8_t *n, unsigned int len)
{
    unsigned int i = 0;
    do {
        if (n[i] < 255) {
            ++n[i];
            break;
        } else {
            n[i] = ~n[i];
        }
    } while (i++ <= len);
}

/* increment */
void BN_inc(BN *bn) {
    BN_expand_maybe(bn);
    BN_unfix(bn);
    BN_inc_raw(bn->n, bn->top);
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

/* number copy */
void BN_copy(BN *res, BN *a) {
    unsigned int i;
    BN_expand(res, a->len);
    res->top = a->top;
    for (i = 0; i <= a->top; i++) {
        res->n[i] = a->n[i];
    }
}

/* const zero */
BN *BN_zero() {
    static const unsigned char zero_[] = { 0x00 };
    static BN zero = { 0, 1, NULL };
    zero.n = (unsigned char*)zero_;
    return &zero;
}

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

/* addition */
void BN_add(BN *result, BN *a, BN *b) {
    unsigned int max_top = (a->top > b->top) ? a->top : b->top;
    BN_copy(result, a);
    BN_expand(result, max_top + 1);
    BN_unfix(result);

    unsigned int i = 0;
    while (i <= b->top) {
        uint8_t carry = result->n[i] & 0x80 || b->n[i] & 0x80;

        result->n[i] += b->n[i];
        if (carry) {
            BN_inc_raw(result->n + i + 1, result->top - i - 1);
        }

        i++;
    }

    BN_fix(result);
}

void BN_add_u8(BN *result, BN *a, uint8_t i) {
    uint8_t carry = 0;

    BN_copy(result, a);
    BN_expand_maybe(result);
    BN_unfix(result);

    if (i & 0x80 || a->n[0] & 0x80) {
        carry = 1;
    }

    result->n[0] += i;
    if (carry) {
        BN_inc_raw(result->n + 1, result->top - 1);
    }

    BN_fix(result);
}

/* subtraction */
void BN_sub(BN *result, BN *a, BN *b) {
    BN_copy(result, a);

    int i = b->top;
    while (i >= 0) {
        uint8_t borrow = b->n[i] > result->n[i];

        result->n[i] -= b->n[i];
        if (borrow) {
            BN_dec_raw(result->n + i + 1, result->top - i - 1);
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
        BN_dec_raw(result->n + 1, result->top - 1);
    } else {
        result->n[0] -= i;
    }

    BN_fix(result);
}

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

int main(int argc, char *argv[]) {
    BN *a = BN_new_from_hex("feff");
    for (unsigned int i = 0; i < 17; i++)
        BN_dec(a);
    BN_print(a);

    BN *b = BN_new_from_hex("0f10");
    for (unsigned int i = 0; i < 20000; i++)
        BN_inc(b);
    BN_print(b);

    printf("a <=> b :: %i\n", BN_cmp(a, b));
    printf("b <=> a :: %i\n", BN_cmp(b, a));
    printf("a <=> 0 :: %i\n", BN_cmp(a, BN_zero()));
    printf("b <=> 0 :: %i\n", BN_cmp(b, BN_zero()));
    printf("0 <=> 0 :: %i\n", BN_cmp(BN_zero(), BN_zero()));

    BN *c = BN_new();
    BN_add(c, a, b);
    BN_print(c);
    BN_free(c);

    BN *d = BN_new();
    BN_sub(d, a, b);
    BN_print(d);
    BN_free(d);

    BN *e = BN_new_from_hex("f");
    BN_print(e);
    BN_free(e);

    BN_free(a);
    BN_free(b);


    /* */
    printf("-----------------\n\n");
    BN *x = BN_new_from_hex("ff");
    BN *r = BN_new();
    BN_add_u8(r, x, 32);
    BN_print(r);
    BN_sub_u8(x, r, 32);
    BN_print(x);
    BN_free(x);
    BN_free(r);

    return 0;
}
