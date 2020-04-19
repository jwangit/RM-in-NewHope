#ifndef INDCPA_H
#define INDCPA_H
#include "poly.h"
#include "../reedmuller/vector.h"
void cpapke_keypair(unsigned char *pk, 
                    unsigned char *sk);

void cpapke_enc(unsigned char *c,
               const unsigned char *m,
               const unsigned char *pk,
               const unsigned char *coins);
vector* cpapke_encRM(unsigned char *c,
               const unsigned char *m,
               const unsigned char *pk,
               const unsigned char *coins);               
void cpapke_enc_wocompr(poly *uhat,
                poly *vprime,
                const unsigned char *m,
                const unsigned char *pk,
                const unsigned char *coin);
vector* cpapke_encRM_wocompr( poly *uhat,
                poly *vprime,
                const unsigned char *m,
                const unsigned char *pk,
                const unsigned char *coin);
void cpapke_dec(unsigned char *m,
               const unsigned char *c,
               const unsigned char *sk);
int cpapke_decRM(vector *decoded,//unsigned char *m,
               const unsigned char *c,
               const unsigned char *sk);
               void cpapke_dec_wocompr(unsigned char *m,
                poly *uhat,
                poly *vprime,
                const unsigned char *sk);
int cpapke_decRM_wocompr(vector *decoded,//unsigned char *m,
                poly *uhat,
                poly *vprime,
                const unsigned char *sk);

#endif
