/* rmencode.c
 *
 * Command-line Reed-Muller encoding utility.
 *
 * By Sebastian Raaphorst, 2002
 * ID#: 1256241
 *
 * $Author: vorpal $
 * $Date: 2002/12/09 04:06:59 $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "reedmuller.h"
#include "common.h"
#include <time.h>
#include "reedmullergmc.h"
#include "vector.h"
//#define OUTPUTINPUT
 reedmuller rm = 0;
 int *message = 0;
 int *codeword = 0;
 int ret_val = 0;


void cleanup()
{
  reedmuller_free(rm);
  free(message);
  free(codeword);
}


int main()
{
  int i, j;
  int r, m;
  time_t t;
  srand((unsigned) time(&t));

  /* try to create the reed-muller code and the vectors */
  r = 4;
  m = 10;
  if ((!(rm = reedmuller_init(r, m)))
      || (!(message = (int*) calloc(rm->k, sizeof(int))))
      || (!(codeword = (int*) calloc(rm->n, sizeof(int))))) {
    fprintf(stderr, "out of memory\n");
    cleanup();
    exit(EXIT_FAILURE);
  }

#ifdef OUTPUTINPUT
  printf("Code parameters for R(%d,%d): n=%d, k=%d\n",
	 rm->r, rm->m, rm->n, rm->k);
  printf("The generator matrix is:\n");
  for (i=0; i < rm->k; ++i) {
    printf("\t");
    for (j=0; j < rm->n; ++j)
      printf("%d ", rm->G[j][i]);
    printf("\n");
  }
  printf("\n");
#endif
  t = clock();
    /* read in the message */
  for (unsigned int l = 0; l < rm->k; l++) {
		message[i] = rand() % 2;
	}
#ifdef OUTPUTINPUT
    for (j=0; j < rm->k; ++j)
      printf("%d", message[j]);
    printf(" -> ");
#endif

    /* encode it */
    reedmuller_encode(rm, message, codeword);
#ifdef OUTPUTINPUT
    for (j=0; j < rm->n; ++j)
      printf("%d", codeword[j]);
    printf("\n");
#endif

/* ================================================================================
	test GMC decoder
================================================================================ */
    Btree *T=NULL;
    double *ptr = (double*)malloc(sizeof(double)*(rm->n)); 
//    printf("REED MULLER CODES RM(%d,%d) \n", par_r,par_m);
//    printf("received Y is ");    
    for (size_t i = 0; i < rm->n; i++)
    {
        ptr[i] = (double)(1.0-2*(codeword[i]))+0.1 ;
    //    printf("%5.1f ",ptr[i]);
    }
    printf("\n");

    T = createTree(ptr, rm->r,  rm->m);

    if (ret_val = vectors_compare(T->chat,codeword,rm->n) !=0)
    {
      printf("GMC decoding errors happen!\n");
    }
    
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("The GMC took %f seconds to execute", time_taken);
 //   preorder(T);
    destroyTree(T);
    
  cleanup();
  exit(EXIT_SUCCESS);
}


/*
 * $Log: rmencode.c,v $
 * Revision 1.3  2002/12/09 04:06:59  vorpal
 * Added changes to allow for decoding.
 * Still have to write rmdecode.c and test.
 *
 * Revision 1.2  2002/11/14 21:05:41  vorpal
 * Tidied up vector reading, and recompiled without debugging defines.
 *
 * Revision 1.1  2002/11/14 21:02:34  vorpal
 * Fixed bugs in reedmuller.c and added command-line encoding app.
 *
 */

