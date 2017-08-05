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
    return (bn->top == bn->len) ? (bn->n[bn->top] & 0x80) : 0;
}

void BN_expand_maybe(BN *bn)
{
    if (BN_top_set(bn)) {
        BN_expand(bn, bn->len + 1);
    }
}

/* decrement */
void BN_dec(BN *bn) {
    unsigned int i = 0;
    do {
        if (bn->n[i] > 0) {
            --bn->n[i];
            break;
        } else {
            bn->n[i] = ~bn->n[i];
        }
    } while (i++ <= bn->top);
    BN_fix(bn);
}

/* increment */
void BN_inc(BN *bn) {
    unsigned int i = 0;

    BN_expand_maybe(bn);
    BN_unfix(bn);

    do {
        if (bn->n[i] < 255) {
            ++bn->n[i];
            break;
        } else {
            bn->n[i] = ~bn->n[i];
        }
    } while (i++ <= bn->top);
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

static int max(int a, int b) {
    if (a >= b)
        return a;
    return b;
}

/* const zero */
BN *BN_zero() {
    static const unsigned char zero_[] = { 0x00 };
    static BN zero = { 0, 1, NULL };
    zero.n = (unsigned char*)zero_;
    return &zero;
}

/* addition */
void BN_add(BN *result, BN *a, BN *b) {
    BN *counter = BN_new();
    BN_copy(counter, b);

    BN_copy(result, a);
    if (BN_top_set(a) && BN_top_set(b)) {
        BN_expand(result, result->top + 1);
    }

    while (BN_cmp(counter, BN_zero()) > 0) {
        BN_inc(result);
        BN_dec(counter);
    }
    BN_free(counter);
}

void BN_add_u8(BN *result, BN *a, uint8_t i) {
    BN_copy(result, a);
    BN_expand(result, result->top + 1);

    // 
}

/* subtraction */
void BN_sub(BN *result, BN *a, BN *b) {
    BN *counter = BN_new();
    BN_copy(counter, b);

    BN_copy(result, a);
    BN_expand(result, result->top + 1);

    while (BN_cmp(counter, BN_zero()) > 0) {
        BN_dec(result);
        BN_dec(counter);
    }
    BN_free(counter);
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

    return 0;
}
