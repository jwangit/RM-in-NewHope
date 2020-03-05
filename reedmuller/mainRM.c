#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "reedmullergmc.h"
#include <time.h>
#include "vector.h"
#include "reedmuller.h"


void test(int,int);

int main()
{
/* ================================================================================
	test RM decoder
================================================================================ */
    setbuf(stdout,NULL);
    time_t t; 

    /* Intializes random number generator */
    srand((unsigned) time(&t));
    int par_r = 4;
    int par_m = 9;
    int par_n = 1 << par_m;
	int i, par_k = 1, d,s, max;
    	for (i = 0, s = 1; i < par_r; i++) {
		s = s * (par_m - i)/(i + 1);
		par_k += s;
	}
	max = 1 << par_n;
	d = par_n >> par_r;
	vector *message, *decoded, *encoded;
	message = create_randvector(par_k);
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
        ptr[i] = (double)(1.0-2*(encoded->values[i])) ;
    //    printf("%5.1f ",ptr[i]);
    }
    printf("\n");
    clock_t clk;
    t = clock();
    T = createTree(ptr, par_r,  par_m);
    t = clock() - t;
   double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
   printf("The GMC took %f seconds to execute", time_taken);
 //   preorder(T);
    destroyTree(T);
    return 0;
}