#ifndef POLY_H
#define POLY_H

#include <stdint.h>
#include "params.h"
#include "../reedmuller/vector.h"
/* 
 * Elements of R_q = Z_q[X]/(X^n + 1). Represents polynomial
 * coeffs[0] + X*coeffs[1] + X^2*xoeffs[2] + ... + X^{n-1}*coeffs[n-1] 
 */
typedef struct {
  uint16_t coeffs[NEWHOPE_N];
} poly __attribute__ ((aligned (32)));

void poly_uniform(poly *a, const unsigned char *seed);
void poly_sample(poly *r, const unsigned char *seed, unsigned char nonce);
void poly_add(poly *r, const poly *a, const poly *b);

void poly_ntt(poly *r);
void poly_invntt(poly *r);
void poly_mul_pointwise(poly *r, const poly *a, const poly *b);

void poly_frombytes(poly *r, const unsigned char *a);
void poly_tobytes(unsigned char *r, const poly *p);
void poly_compress(unsigned char *r, const poly *p);
void poly_decompress(poly *r, const unsigned char *a);

void poly_frommsg(poly *r, const unsigned char *msg);
void poly_tomsg(unsigned char *msg, const poly *x);
void poly_sub(poly *r, const poly *a, const poly *b);
//jwang
void poly_sampleK(poly *r, const unsigned char *seed, unsigned char nonce);
void poly_sampleKmodif(poly *r, const unsigned char *seed, unsigned char nonce);
vector* poly_fromRM(poly *r, const unsigned char *msg, int par_r, int par_m, int par_k);
void poly_toRM(vector *decoded, const poly *x, int par_r, int par_m, int par_N);
void poly_tomsgdecisn(unsigned char *msg, const poly *x, int16_t tdecisn[]);
void poly_toRMdebug(vector *decoded, const poly *x, int par_r, int par_m, int par_N, double inputGMC[]);

#endif
