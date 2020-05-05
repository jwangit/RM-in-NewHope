//
//  mainRM.c
//  
//
//  Created by Jiabo Wang on 12/03/20.
//  Copyright (c) 2020 Jiabo Wang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "reedmullergmc.h"
#include <time.h>
#include "vector.h"
#include "reedmuller.h"


int main()
{
    int x = 9217;
    uint16_t y = x;
    printf("%d\n",y);
/* ================================================================================
	test RM decoder
================================================================================ */
    int ret_val = 0;
    setbuf(stdout,NULL);
    time_t t; 

    /* Intializes random number generator */
    srand((unsigned) time(&t));
    t = clock();
    int par_r = 1;
    int par_m = 4;
    int par_n = 1 << par_m;
	int i, par_k = 1, d,s, max;
    	for (i = 0, s = 1; i < par_r; i++) {
		s = s * (par_m - i)/(i + 1);
		par_k += s;
	}
	max = 1 << par_n;
	d = par_n >> par_r;
	vector *message, *encoded;
	message = create_randvector(par_k);
    vector *decoded = (vector *)malloc(sizeof(vector));
    decoded->length = par_n;
    decoded->values = (int *) malloc(sizeof(int)*par_n);
/*    printf("Message is: ");
    for (int i = 0; i < (message->length); i++)
    {
        printf("%d ", message->values[i]);        
    }
    printf("\n");
    printf("generator matrix : \n");*/
    encoded = encode(message,par_r, par_m);
/*    printf("\n");

    printf("Codeword is: ");
    for (int i = 0; i < (encoded->length); i++)
    {
        printf("%d ", encoded->values[i]);        
    }
    printf("\n");*/
    

/* ================================================================================
	test GMC decoder
================================================================================ */
    Btree *T=NULL;
    double *ptr = (double*)malloc(sizeof(double)*par_n); 
//    printf("REED MULLER CODES RM(%d,%d) \n", par_r,par_m);
//    printf("received Y is ");    
    for (size_t i = 0; i < par_n; i++)
    {
        ptr[i] = (double)(1.0-2*(encoded->values[i]))+0.1 ;
    //    printf("%5.1f ",ptr[i]);
    }
    printf("\n");

    T = createTree(ptr, par_r,  par_m);
    
    for ( unsigned int i = 0; i < par_n; i++)
    {
        decoded->values[i] = T->chat[i];
    }
    
    if(ret_val = compare_vectors(encoded, decoded) != 0)
        {
  
            printf("crypto_kem_dec returned bad 'RM' codeword\n");
//           return KAT_CRYPTO_FAILURE;
        }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("The GMC took %f seconds to execute", time_taken);
 //   preorder(T);
    destroyTree(T);
    destroy_vector(message);
    destroy_vector(encoded);
    return 0;
}