#ifndef INDCPA_H
#define INDCPA_H
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

void cpapke_dec(unsigned char *m,
               const unsigned char *c,
               const unsigned char *sk);
int cpapke_decRM(vector *decoded,//unsigned char *m,
               const unsigned char *c,
               const unsigned char *sk);

#endif
