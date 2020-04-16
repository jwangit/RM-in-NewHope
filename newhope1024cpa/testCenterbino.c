#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "api.h"
#include "poly.h"
#include "rng.h"
#include "fips202.h"
#include "time.h"

#define TXT_FILE_OPEN_ERROR -1
#define KAT_FILE_OPEN_ERROR -2
#define	MAX_MARKER_LEN		50
int		FindMarker(FILE *infile, const char *marker);
int		ReadHex(FILE *infile, unsigned char *A, int Length, char *str);
int
main()
{
    FILE    *fp_centrbino, *fp_req;
    unsigned int count;
    unsigned int done = 0;
    unsigned char seed[48];
    char    fn_req[32],fn_centrbino[32];
    sprintf(fn_req, "PQCkemKAT_%d.req", NEWHOPE_N);
    sprintf(fn_centrbino,"centrbinoK_%d.txt",NEWHOPE_bytesofK*8+2*NEWHOPE_numof2bits);

    //Create the RESPONSE file based on what's in the REQUEST file
    if ( (fp_req = fopen(fn_req, "r")) == NULL ) {
        printf("Couldn't open <%s> for read\n", fn_req);
        return KAT_FILE_OPEN_ERROR;
    }
    if ( (fp_centrbino = fopen(fn_centrbino,"w+")) == NULL )
    {
        printf("Couldn't open bino.txt!\n");
        return TXT_FILE_OPEN_ERROR;
    }
    
    poly r;
    unsigned char z[2*NEWHOPE_SYMBYTES];
    unsigned char *noiseseed = z+NEWHOPE_SYMBYTES;
    do
    {

        if ( FindMarker(fp_req, "count = ") )
        {
            fscanf(fp_req, "%d", &count);
           /* if (count == 10)
            {
                done = 1;
                break;
            }*/
        }
        else {
            done = 1;
            break;
        }
        
        if ( !ReadHex(fp_req, seed, 48, "seed = ") ) {
            printf("ERROR: unable to read 'seed' from <%s>\n", fn_req);
            return KAT_FILE_OPEN_ERROR;
        }
		randombytes_init(seed, NULL, 256);
        randombytes(z, NEWHOPE_SYMBYTES);
        shake256(z, 2*NEWHOPE_SYMBYTES, z, NEWHOPE_SYMBYTES);
        poly_sampleKmodif(&r, noiseseed, 0);
        for (unsigned int i = 0; i < NEWHOPE_N; i++)
        {
            fprintf(fp_centrbino,"%d ",r.coeffs[i]-NEWHOPE_Q);
        }

    } while (!done);    
    fclose(fp_centrbino);
    fclose(fp_req);
    
    return 0;
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