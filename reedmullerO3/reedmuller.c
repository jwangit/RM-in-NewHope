/* reedmuller.c
 *
 * By Sebastian Raaphorst, 2002
 * ID#: 1256241
 *
 * $Author: vorpal $
 * $Date: 2002/12/09 15:07:04 $
 */

#include <stdlib.h>
#include "matrix.h"
#include "ksubset.h"
#include "vector.h"
#include "common.h"
#include "reedmuller.h"


reedmuller reedmuller_init(int r, int m)
{
  int i, j, k;
  int bound;
  int flg;
  int *tmpptr;
  reedmuller rm;

  rm = 0;
  if (!(rm = (reedmuller) malloc(sizeof(struct _reedmuller)))) {
    reedmuller_free(rm);
    return FALSE;
  }

  rm->r = r;
  rm->m = m;
  rm->q = 2;

  /* zero data structures in rm */
  rm->s    = 0;
  rm->G    = 0;
  rm->rows = 0;
  for (i=0; i < NUMVECTORS; ++i)
    rm->vector[i] = 0;
  for (i=0; i < NUMSUBSETS; ++i)
    rm->subset[i] = 0;

  /* now we need to calculate n and k */
  /* n is trivial: q^m - we are currently only working over the finite field
     of order 2, so we do this efficiently */
  rm->n = (1 << m);

  /* create the set */
  if (!(rm->s = ksubset_init(rm->m))) {
    reedmuller_free(rm);
    return FALSE;
  }

  /* calculate k */
  for (i=0, rm->k=0; i <= r; ++i)
    rm->k += rm->s->nCr[rm->m][i];

  /* create the generator matrix, which stores columns of values */
  if (!(rm->G = matrix_2d(rm->n, rm->k))) {
    reedmuller_free(rm);
    return FALSE;
  }

  /* create the row matrix, which stores rows of values */
  if (!(rm->rows = matrix_2d(rm->k, rm->n))) {
    reedmuller_free(rm);
    return FALSE;
  }

  /* create the vectors and the subsets */
  for (i=0; i < NUMVECTORS; ++i)
    if (!(rm->vector[i] = (int*) calloc(rm->n, sizeof(int)))) {
      reedmuller_free(rm);
      return FALSE;
    }
  for (i=0; i < NUMSUBSETS; ++i)
    if (!(rm->subset[i] = (int*) calloc(rm->m, sizeof(int)))) {
      reedmuller_free(rm);
      return FALSE;
    }

  /* now we can process the matrix; set the first row to the 1
     Boolean polynomial, which is all 1s */
  for (i=0; i < rm->n; ++i)
    rm->rows[0][i] = 1;

  if (r > 0) {
    /* special case: iterate over the monomials */
    for (i=1; i <= m; ++i) {
      for (k=0; k < rm->n;) {
	bound = 1 << (m-i);
	for (j=0; j < bound; ++j) {
	  rm->rows[i][k] = 1;
	  ++k;
	}
	for (j=0; j < bound; ++j) {
	  rm->rows[i][k] = 0;
	  ++k;
	}
      }
    }

    /* we have all the monomials; now we can simply
       take k-subsets of elements and calculate the rows
       using vector operations */
    for (k=m+1, i=2; i <= r; ++i) {
      /* get the first subset */
      for (ksubset_lex_unrank(rm->s, i, 0, rm->subset[1]), flg=TRUE;
	   flg;
	   flg = ksubset_lex_succ(rm->s, i, rm->subset[0], rm->subset[1])) {
	/* switch subset 1 and 2 */
	tmpptr = rm->subset[0];
	rm->subset[0] = rm->subset[1];
	rm->subset[1] = tmpptr;

	/* clear row k of our matrix and vector1 */
	for (j=0; j < rm->n; ++j)
	  rm->rows[k][j] = rm->vector[0][j] = 1;

	/* perform vector multiplication, accumulating the result */
	for (j=0; j < i; ++j) {
	  vector_multiply(rm->q, rm->n, rm->rows[rm->subset[0][j]],
			  rm->vector[0], rm->vector[1]);

	  /* swap vector2 and vector 1 */
	  tmpptr = rm->vector[0];
	  rm->vector[0] = rm->vector[1];
	  rm->vector[1] = tmpptr;
	}

	/* copy the contents of vector1 to our matrix */
	vector_copy(rm->n, rm->vector[0], rm->rows[k]);
	++k;
      }
    }
  }

  /* now we convert from an array of rows to an array of columns */
  for (i=0; i < rm->n; ++i)
    for (j=0; j < rm->k; ++j)
      rm->G[i][j] = rm->rows[j][i];

  /* we are done */
  return rm;
}


void reedmuller_free(reedmuller rm)
{
  int i;

  if (rm) {
    for (i=0; i < NUMVECTORS; ++i)
      if (rm->vector[i])
	free(rm->vector[i]);
    for (i=0; i < NUMSUBSETS; ++i)
      if (rm->subset[i])
	free(rm->subset[i]);

    if (rm->s)
      ksubset_free(rm->s);
    if (rm->G)
      matrix_free_2d(rm->G, rm->n, rm->k);
    if (rm->rows)
      matrix_free_2d(rm->rows, rm->k, rm->n);
    free(rm);
  }
}


int reedmuller_encode(reedmuller rm, int *message, int *codeword)
{
  int i;

  /* simply take the dot product between the message and the columns
     of the generator matrix */
  for (i=0; i < rm->n; ++i)
    codeword[i] = vector_dotproduct(rm->q, rm->k, message, rm->G[i]);

  return TRUE;
}


/*
 * $Log: reedmuller.c,v $
 * Revision 1.7  2002/12/09 15:07:04  vorpal
 * Fixed decoding routine so that the received message was not
 * altered during computation (had to add a fourth work vector to
 * the reedmuller struct).
 *
 * Revision 1.6  2002/12/09 15:04:02  vorpal
 * Working!!!
 *
 * Revision 1.5  2002/12/09 14:59:48  vorpal
 * Decoding seems to be working now!
 * More tests needed, though (and debugging statements must be removed).
 *
 * Revision 1.4  2002/12/09 04:25:44  vorpal
 * Fixed some glaring errors in reedmuller.c
 * Still need to fix problems with decoding; not doing it properly.
 *
 * Revision 1.3  2002/12/09 04:06:59  vorpal
 * Added changes to allow for decoding.
 * Still have to write rmdecode.c and test.
 *
 * Revision 1.2  2002/11/14 21:02:34  vorpal
 * Fixed bugs in reedmuller.c and added command-line encoding app.
 *
 * Revision 1.1  2002/11/14 20:28:05  vorpal
 * Adding new files to project.
 *
 */
