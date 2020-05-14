//
//  PQCgenKAT_pkeRC.c
//
//  Created by Jiabo Wang (Mar) on 11/03/20.
//  Copyright © 2020 Wang, Jiabo (Mar). All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rng.h"
#include "fips202.h"
#include "api.h"
#include "cpapke.h"
#include "time.h"
#include "poly.h"

#define	MAX_MARKER_LEN		50
#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_DATA_ERROR      -3
#define KAT_CRYPTO_FAILURE  -4

int		FindMarker(FILE *infile, const char *marker);
int		ReadHex(FILE *infile, unsigned char *A, int Length, char *str);
void	fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L);

int
main()
{
    char                fn_req[32], fn_rsp[32];
    FILE                *fp_req, *fp_rsp;
    unsigned char       seed[48];
    unsigned char       entropy_input[48];
    unsigned char       muhat[CRYPTO_BYTES];//ct[CRYPTO_CIPHERTEXTBYTES],, ss1[CRYPTO_BYTES];
	poly *uhat = (poly *)malloc(sizeof(poly));
	poly *vprime = (poly *)malloc(sizeof(poly));
    poly *ehat = (poly *)malloc(sizeof(poly));
	poly *shat = (poly *)malloc(sizeof(poly));
    poly *sprimehat = (poly *)malloc(sizeof(poly));
    poly *eprimehat = (poly *)malloc(sizeof(poly));
	poly *eprimeprime = (poly *)malloc(sizeof(poly));
    poly *v = (poly *)malloc(sizeof(poly));
    poly *vmsg = (poly *)malloc(sizeof(poly));
    unsigned char z[NEWHOPE_SYMBYTES];
    unsigned char *noiseseed = z;

    int                 count;
	int					done=0;
    unsigned char       pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];
    int                 ret_val;
    unsigned char buf[2*NEWHOPE_SYMBYTES];
    unsigned int framerrCount = 0;
	unsigned int NumofIteration = 1000;
	time_t t;
    int16_t tempdec[256];
    for (unsigned int i = 0; i < 256; i++)
    {
        tempdec[i] = 0;
    }
    
    printf("Working...\n");
	// Create the REQUEST file
    sprintf(fn_req, "PQCkemKAT_%d.req", NEWHOPE_N);
    if ( (fp_req = fopen(fn_req, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_req);
        return KAT_FILE_OPEN_ERROR;
    }
    // Create the RESPONSE file
    sprintf(fn_rsp, "PQCpkeKAT_%d.rsp", NEWHOPE_N);
    if ( (fp_rsp = fopen(fn_rsp, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_rsp);
        return KAT_FILE_OPEN_ERROR;
    }

    //randomness source 
	//srand((unsigned) time(&t));
     for (int i=0; i<48; i++)
        entropy_input[i] = i;//rand()%256;    

    randombytes_init(entropy_input, NULL, 256);
    for (int i=0; i<NumofIteration; i++) {
        fprintf(fp_req, "count = %d\n", i);
        randombytes(seed, 48);
        fprintBstr(fp_req, "seed = ", seed, 48);
    }
    fclose(fp_req);

    //Create the RESPONSE file based on what's in the REQUEST file
    if ( (fp_req = fopen(fn_req, "r")) == NULL ) {
        printf("Couldn't open <%s> for read\n", fn_req);
        return KAT_FILE_OPEN_ERROR;
    }
	fprintf(fp_rsp, "# %s\n\n", CRYPTO_PKE_RP);
	fprintf(fp_rsp, "K = %d\n", NEWHOPE_bytesofK*8+2*NEWHOPE_numof2bits);
    fflush(fp_rsp); 

	int returnfscanf;    
    t = clock();
    do {
		if ( FindMarker(fp_req, "count = ") )
            returnfscanf = fscanf(fp_req, "%d", &count);
        else {
            done = 1;
            break;
        }
        
        if ( !ReadHex(fp_req, seed, 48, "seed = ") ) {
            printf("ERROR: unable to read 'seed' from <%s>\n", fn_req);
            return KAT_DATA_ERROR;
        }
		randombytes_init(seed, NULL, 256);

        // Generate the public/private keypair
        randombytes(z, NEWHOPE_SYMBYTES);
        poly_sampleKmodif(shat, noiseseed, 0);
        poly_ntt(shat);
        poly_sampleKmodif(ehat, noiseseed, 1);
        poly_ntt(ehat);
        /*if ( (ret_val = crypto_kem_keypair(pk, sk)) != 0) {
            printf("crypto_kem_keypair returned <%d>\n", ret_val);
            return KAT_CRYPTO_FAILURE;
        }*/

        
        // NewHope-CPA-PKE ENCRYPTION
        randombytes(buf,NEWHOPE_SYMBYTES);
		shake256(buf,2*NEWHOPE_SYMBYTES,buf,NEWHOPE_SYMBYTES);
        poly_frommsg(v, buf);
        for (unsigned int i = 0; i < NEWHOPE_N; i++)
        {
            vmsg->coeffs[i] = v->coeffs[i];
        }
        
        poly_sampleKmodif(sprimehat, buf+NEWHOPE_SYMBYTES, 0);
        poly_sampleKmodif(eprimehat, buf+NEWHOPE_SYMBYTES, 1);
        poly_sampleKmodif(eprimeprime, buf+NEWHOPE_SYMBYTES, 2);

        poly_ntt(sprimehat);
        poly_ntt(eprimehat);

        poly_mul_pointwise(ehat,ehat,sprimehat);
        poly_mul_pointwise(eprimehat,eprimehat,shat);
        poly_sub(ehat, eprimehat, ehat);
        poly_invntt(ehat);
        poly_add(ehat, ehat, eprimeprime);
        poly_add(v, v,ehat);

        //cpapke_enc_wocompr(uhat, vprime,buf, pk, buf+NEWHOPE_SYMBYTES);       
 
        
        // NewHope-CPA-PKE DECRYPTION 
        poly_tomsgdecisn(muhat, v, tempdec);
/*        for(unsigned int i=0;i<256;i++)
        {
            tempdec  = flipabs(v->coeffs[i+  0]);
            tempdec += flipabs(v->coeffs[i+256]);
            tempdec += flipabs(v->coeffs[i+512]);
            tempdec += flipabs(v->coeffs[i+768]);
            tempdec = ((tempdec - NEWHOPE_Q));
            tempdec >>= 15;
        if ( tempdec != (buf[i>>3]>>(i & 7)))
        {
            fprintf(fp_rsp, "Count = %d, msg = %d, recmsg = %d\n ", count, vmsg->coeffs[i], v->coeffs[i]);            
        }
    
        }*/
        //cpapke_dec_wocompr(muhat, uhat, vprime, sk);
        if ( memcmp(muhat, buf, NEWHOPE_SYMBYTES) ) {
            framerrCount ++;
			//printf("crypto_kem_dec returned bad 'ss' value\n");
            //return KAT_CRYPTO_FAILURE;
        }
        for (unsigned int i = 0; i < 256; i++)
        {
            if ( ((muhat[i>>3] >> (i & 7)) & 1) != ((buf[i>>3] >> (i & 7)) & 1) )
            {
                fprintf(fp_rsp, "Count = %d, index = %d, i>>3=%d, i&7=%d\n", count, i, (i>>3), (i&7)); 
                fprintf(fp_rsp, "hrdecision = %d\n", tempdec[i]);
                fprintf(fp_rsp, "sntmsg = %d, %d, %d, %d\n", vmsg->coeffs[i], vmsg->coeffs[i+256],vmsg->coeffs[i+512],vmsg->coeffs[i+768]); 
                fprintf(fp_rsp, "recmsg = %d, %d, %d, %d\n\n", v->coeffs[i], v->coeffs[i+256],v->coeffs[i+512],v->coeffs[i+768]); 
                fflush(fp_rsp);
            }
                 
        }
        
        
    } while ( !done );
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("The GMC took %f seconds to execute\n", time_taken);

    fprintf(fp_rsp, "framerrCount = %d,     ", framerrCount);
	fprintf(fp_rsp, "TotlframCount = %d,     ", count+1);
	fprintf(fp_rsp, "framerrRate = %e\n", (1.0*framerrCount/NumofIteration) );
	fflush(fp_rsp); 
    fclose(fp_rsp); 
	fclose(fp_req);
    return KAT_SUCCESS;
}



//
// ALLOW TO READ HEXADECIMAL ENTRY (KEYS, DATA, TEXT, etc.)
//
//
// ALLOW TO READ HEXADECIMAL ENTRY (KEYS, DATA, TEXT, etc.)
//
int
FindMarker(FILE *infile, const char *marker)
{
	char	line[MAX_MARKER_LEN];
	int		i, len;
	int curr_line;

	len = (int)strlen(marker);
	if ( len > MAX_MARKER_LEN-1 )
		len = MAX_MARKER_LEN-1;

	for ( i=0; i<len; i++ )
	  {
	    curr_line = fgetc(infile);
	    line[i] = curr_line;
	    if (curr_line == EOF )
	      return 0;
	  }
	line[len] = '\0';

	while ( 1 ) {
		if ( !strncmp(line, marker, len) )
			return 1;

		for ( i=0; i<len-1; i++ )
			line[i] = line[i+1];
		curr_line = fgetc(infile);
		line[len-1] = curr_line;
		if (curr_line == EOF )
		    return 0;
		line[len] = '\0';
	}

	// shouldn't get here
	return 0;
}

//
// ALLOW TO READ HEXADECIMAL ENTRY (KEYS, DATA, TEXT, etc.)
//
int
ReadHex(FILE *infile, unsigned char *A, int Length, char *str)
{
	int			i, ch, started;
	unsigned char	ich;

	if ( Length == 0 ) {
		A[0] = 0x00;
		return 1;
	}
	memset(A, 0x00, Length);
	started = 0;
	if ( FindMarker(infile, str) )
		while ( (ch = fgetc(infile)) != EOF ) {
			if ( !isxdigit(ch) ) {
				if ( !started ) {
					if ( ch == '\n' )
						break;
					else
						continue;
				}
				else
					break;
			}
			started = 1;
			if ( (ch >= '0') && (ch <= '9') )
				ich = ch - '0';
			else if ( (ch >= 'A') && (ch <= 'F') )
				ich = ch - 'A' + 10;
			else if ( (ch >= 'a') && (ch <= 'f') )
				ich = ch - 'a' + 10;
            else // shouldn't ever get here
                ich = 0;
			
			for ( i=0; i<Length-1; i++ )
				A[i] = (A[i] << 4) | (A[i+1] >> 4);
			A[Length-1] = (A[Length-1] << 4) | ich;
		}
	else
		return 0;

	return 1;
}

void
fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L)
{
	unsigned long long  i;

	fprintf(fp, "%s", S);

	for ( i=0; i<L; i++ )
		fprintf(fp, "%02X", A[i]);

	if ( L == 0 )
		fprintf(fp, "00");

	fprintf(fp, "\n");
}

