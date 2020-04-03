#ifndef API_H
#define API_H

#include "params.h"

#define CRYPTO_SECRETKEYBYTES  NEWHOPE_CPAKEM_SECRETKEYBYTES
#define CRYPTO_PUBLICKEYBYTES  NEWHOPE_CPAKEM_PUBLICKEYBYTES
#define CRYPTO_CIPHERTEXTBYTES NEWHOPE_CPAKEM_CIPHERTEXTBYTES
#define CRYPTO_BYTES           NEWHOPE_SYMBYTES

#if   (NEWHOPE_N == 512)
#define CRYPTO_ALGNAME "NewHope512-CPAKEM"
#define CRYPTO_PKE_RP "NewHope512-CPAPKE-REPETITIONCODES"
#define CRYPTO_PKE_RM "NewHope512-CPAPKE-RM"
#define RM_r				4
#define RM_m				9
#define RM_k				256
#define RM_N				512
#elif (NEWHOPE_N == 1024)
#define CRYPTO_ALGNAME "NewHope1024-CPAKEM"
#define CRYPTO_PKE_RP "NewHope1024-CPAPKE-REPETITIONCODES"
#define CRYPTO_PKE_RM "NewHope1024-CPAPKE-RM"
#define RM_r				4
#define RM_m				10
#define RM_k				386
#define RM_N				1024
#else
#error "NEWHOPE_N must be either 512 or 1024"
#endif

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);

int crypto_kem_enc(unsigned char *ct, unsigned char *ss, const unsigned char *pk);

int crypto_kem_dec(unsigned char *ss, const unsigned char *ct, const unsigned char *sk);

#endif
