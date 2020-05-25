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
#include "math.h"
#include "../reedmuller/reedmullergmc.h"

#define	MAX_MARKER_LEN		50
#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1

int		FindMarker(FILE *infile, const char *marker);
int		ReadHex(FILE *infile, unsigned char *A, int Length, char *str);

int
main()
{
	//save newhope additive noise
    char                fn_rsp[32],fn_errlogdetl[32], fn_errlog[32];
    FILE                *fp_errlog, *fp_errlogdetl, *fp_rsp;

    poly *ehat = (poly *)malloc(sizeof(poly));
	poly *shat = (poly *)malloc(sizeof(poly));
    poly *sprimehat = (poly *)malloc(sizeof(poly));
    poly *eprimehat = (poly *)malloc(sizeof(poly));
	poly *eprimeprime = (poly *)malloc(sizeof(poly));
    poly *v = (poly *)malloc(sizeof(poly));
//    poly *vmsg = (poly *)malloc(sizeof(poly));
    unsigned char noiseseed[NEWHOPE_SYMBYTES] ;

    int                 count = 0;
    int                 ret_val;
    unsigned char buf[49+NEWHOPE_SYMBYTES];
    unsigned int framerrCount = 0;
	unsigned int NumofIteration = 100;
    vector *encoded;
    vector *decoded = (vector *)malloc(sizeof(vector));
    decoded->length = NEWHOPE_N;
    decoded->values = (int *) malloc(sizeof(int)*NEWHOPE_N);
	time_t t;
    int16_t tempdec[386];
    Btree* T;
    for (unsigned int i = 0; i < 386; i++)
    {
        tempdec[i] = 0;
    }
    double deucl = 0;
    double inputGMC[NEWHOPE_N];
    for (unsigned int i = 0; i < NEWHOPE_N; i++)
    {
        inputGMC[i] = 0;
    }

    printf("Working...\n");
	//save newhope additive noise
	sprintf(fn_errlog,"RM%derrlog%d.txt",NEWHOPE_N,NEWHOPE_bytesofK*8+2*NEWHOPE_numof2bits);
    if ( (fp_errlog = fopen(fn_errlog, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_errlog);
        return KAT_FILE_OPEN_ERROR;
    }
    sprintf(fn_errlogdetl,"RM%derrlogdetl%d.txt",NEWHOPE_N,NEWHOPE_bytesofK*8+2*NEWHOPE_numof2bits);
    if ( (fp_errlogdetl = fopen(fn_errlogdetl, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_errlogdetl);
        return KAT_FILE_OPEN_ERROR;
    }
    // Create the RESPONSE file
    sprintf(fn_rsp, "PQCpkeKAT_%d.rsp", NEWHOPE_N);
	if ( (fp_rsp = fopen(fn_rsp, "w+")) == NULL ) {
        printf("Couldn't open <%s> for write\n", fn_rsp);
        return KAT_FILE_OPEN_ERROR;
    }
	fprintf(fp_rsp, "# %s\n\n", CRYPTO_PKE_RM);
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
        for (int i=0; i<81; i++)
            buf[i] = rand()%256;  
		encoded = poly_fromRM(v, buf, RM_r, RM_m, RM_k);

        poly_sampleKmodif(sprimehat, buf+49, 0);
        poly_sampleKmodif(eprimehat, buf+49, 1);
        poly_sampleKmodif(eprimeprime, buf+49, 2);

        poly_ntt(sprimehat);
        poly_ntt(eprimehat);

        poly_mul_pointwise(ehat,ehat,sprimehat);
        poly_mul_pointwise(eprimehat,eprimehat,shat);
        poly_sub(ehat, eprimehat, ehat);
        poly_invntt(ehat);
        poly_add(ehat, ehat, eprimeprime);
        poly_add(v, v,ehat);
		       
        // NewHope-CPA-PKE DECRYPTION 
        T = poly_toRMdebug(decoded,v, RM_r, RM_m, RM_N, inputGMC);
        for (unsigned int k = 0; k < RM_N; k++)
        {
            deucl += (inputGMC[k]-(1.0-2.0*(encoded->values[k])))*(inputGMC[k]-(1.0-2.0*(encoded->values[k])));
//                fprintf(fp_errlog,"%f ", inputGMC[k]);
//                fprintf(fp_errlog,"%f ", inputGMC[k]);
        }
        /*fprintf(fp_errlog,"count = %d, ", count);
        fprintf(fp_errlog,"d^2 = %f, ", deucl);*/
        fprintf(fp_errlog,"%d ", count);
        fprintf(fp_errlog,"%5.3f ", deucl);
        deucl = 0;
        if(ret_val = compare_vectors(encoded, decoded) != 0)
        {
    		framerrCount ++;
            fprintf(fp_errlog,"1\n");
            travBTree(T,fp_errlogdetl);
        }else
        {
            fprintf(fp_errlog,"0\n");
        }
		destroy_vector(encoded);
        destroyTree(T);////////////////////////////25/05/2020 jwang
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
	fclose(fp_errlog);
	fclose(fp_errlogdetl);
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



