/*
 * not-so-bignum library
 * Copyright (C) 2017-2019 Aaron Marks. All Rights Reserved.
 */
#ifndef NBN_H
#define NBN_H

#include <stdint.h>
#include <ctype.h>
#include <assert.h>

typedef struct {
    unsigned int top;
    unsigned int len;
    uint8_t *n;
} BN;

BN *BN_new();
void BN_free(BN *bn);
void BN_fix(BN *bn);
void BN_unfix(BN *bn);
int BN_expand(BN *bn, unsigned int size);
int BN_top_set(BN *bn);
void BN_dec(BN *bn);
void BN_inc(BN *bn);
int BN_cmp(BN *a, BN *b);
void BN_copy(BN *res, BN *a);
BN *BN_max(BN *a, BN *b);
BN *BN_min(BN *a, BN *b);
void BN_shl_u8(BN *result, BN *a, uint8_t shl);
void BN_shl_u64(BN *result, BN *a, uint64_t shl);
void BN_shr_u8(BN *result, BN *a, uint8_t shr);
void BN_shr_u64(BN *result, BN *a, uint64_t shr);
void BN_add(BN *result, BN *a, BN *b);
void BN_add_u8(BN *result, BN *a, uint8_t i);
void BN_sub(BN *result, BN *a, BN *b);
void BN_sub_u8(BN *result, BN *a, uint8_t i);
void BN_mul(BN *result, BN *a, BN *b);
void BN_div(BN *q, BN *r, BN *n, BN *d);
void BN_print(BN *bn);
int BN_to_string(BN *bn, char *str, size_t str_len);

int BN_from_hex(BN *bn, char *str);
BN *BN_new_from_hex(char *str);

void BN_dec_raw(uint8_t *n, unsigned int len);
void BN_inc_raw(uint8_t *n, unsigned int len);
void BN_shl_raw(uint8_t *n, unsigned int len, uint64_t shift);
void BN_shr_raw(uint8_t *n, unsigned int len, uint64_t shift);

#define BN_const_declare(name)                  \
    BN *BN_##name(void)
BN_const_declare(zero);
BN_const_declare(one);
BN_const_declare(two);
BN_const_declare(three);
BN_const_declare(four);
BN_const_declare(five);
BN_const_declare(six);
BN_const_declare(seven);
BN_const_declare(eight);
BN_const_declare(nine);
BN_const_declare(ten);



#endif
