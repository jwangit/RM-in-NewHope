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

int		FindMarker(FILE *infile, const char *marker);
int		ReadHex(FILE *infile, unsigned char *A, int Length, char *str);

int
main()
{
	//save newhope additive noise
    char                fn_rsp[32],fn_nhnoise[32];
    FILE                *fp_nhnoise, *fp_rsp;
    unsigned char       muhat[CRYPTO_BYTES];//ct[CRYPTO_CIPHERTEXTBYTES],, ss1[CRYPTO_BYTES];

    poly *ehat = (poly *)malloc(sizeof(poly));
	poly *shat = (poly *)malloc(sizeof(poly));
    poly *sprimehat = (poly *)malloc(sizeof(poly));
    poly *eprimehat = (poly *)malloc(sizeof(poly));
	poly *eprimeprime = (poly *)malloc(sizeof(poly));
    poly *v = (poly *)malloc(sizeof(poly));
    unsigned char noiseseed[NEWHOPE_SYMBYTES] ;

    int                 count = 0;
    int                 ret_val;
    unsigned char buf[2*NEWHOPE_SYMBYTES];
    unsigned int framerrCount = 0;
	unsigned int NumofIteration = 10000;
	time_t t;

    printf("Working...\n");
	//save newhope additive noise
	sprintf(fn_nhnoise,"RCnhnoise%d.txt",NEWHOPE_bytesofK*8+2*NEWHOPE_numof2bits);
    if ( (fp_nhnoise = fopen(fn_nhnoise, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_nhnoise);
        return KAT_FILE_OPEN_ERROR;
    }
    // Create the RESPONSE file
    sprintf(fn_rsp, "PQCpkeKAT_%d.rsp", NEWHOPE_N);
	if ( (fp_rsp = fopen(fn_rsp, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_rsp);
        return KAT_FILE_OPEN_ERROR;
    }
	fprintf(fp_rsp, "# %s\n\n", CRYPTO_PKE_RP);
	fprintf(fp_rsp, "K = %d,     ", NEWHOPE_bytesofK*8+2*NEWHOPE_numof2bits);
    fflush(fp_rsp); 
	
    //randomness source 
	srand((unsigned) time(&t));

	int returnfscanf;    
    t = clock();
    do {
        count++;
        // Generate the public/private keypair
        //randombytes(z, NEWHOPE_SYMBYTES);
        for (int i=0; i<32; i++)
            noiseseed[i] = rand()%256;  
        poly_sampleKmodif(shat, noiseseed, 0);
        poly_ntt(shat);
        poly_sampleKmodif(ehat, noiseseed, 1);
        poly_ntt(ehat);
        
        // NewHope-CPA-PKE ENCRYPTION
        for (int i=0; i<64; i++)
            buf[i] = rand()%256;  
        poly_frommsg(v, buf);
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
		//save newhope additive noise
        for (unsigned int i = 0; i < NEWHOPE_N; i++)
        {
            fprintf(fp_nhnoise,"%d ",v->coeffs[i]);
        }
        // NewHope-CPA-PKE DECRYPTION 
        poly_tomsg(muhat, v);


        if ( memcmp(muhat, buf, NEWHOPE_SYMBYTES) ) {
            framerrCount ++;
        }
        
    } while ( count <NumofIteration );
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("The GMC took %f seconds to execute\n", time_taken);
    fprintf(fp_rsp, "framerrCount = %d,     ", framerrCount);
	fprintf(fp_rsp, "TotlframCount = %d,     ", count);
	fprintf(fp_rsp, "framerrRate = %e\n", (1.0*framerrCount/NumofIteration) );
	fflush(fp_rsp); 
    fclose(fp_rsp); 
	//save newhope additive noise
	fclose(fp_nhnoise);
	
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



