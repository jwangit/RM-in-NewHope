//
//  reedmullergmc.c
//  
//
//  Created by Jiabo Wang on 12/03/20.
//  Copyright (c) 2020 Jiabo Wang. All rights reserved.
//


#include "reedmullergmc.h"
#include <math.h>

int sign(double x)
{
	int z;
    z = (x>0) - (x<0);
	return(z);
}
Btree *createTree(double *ptr, int8_t r, int8_t m)
{
    int N = 1 << m;
    int pus =N/2;
/*	int i, dim = 1,s;
    for (i = 0, s = 1; i < r; i++) {
		s = s * (m - i)/(i + 1);
		dim += s;
	}*/

    double Ysum = 0;
    int temp = 0;
    int temp1 = 0;
    double smallestV = 0;
    int smallestInd = 0;
    Btree *T;
    T = (Btree *) malloc(sizeof(Btree));
    T->recYv = (double*) malloc(sizeof(double)*pus);
    T->recYu = (double*) malloc(sizeof(double)*pus);
    T->chat = (int8_t *) malloc(sizeof(int8_t)*N);///////////////////////REMEMBER TO FREE!!!!!!!!!
 //   T->chathat = (vector*) malloc(sizeof(vector));
 //   T->chathat->values = (int *) malloc(sizeof(int)*N);
 //   T->sys = (int8_t *) malloc(sizeof(int8_t)*dim);///////////////////////REMEMBER TO FREE!!!!!!!!!
    (T) -> r = r;
    (T) -> m = m;
//    T->chathat->length = N;
    
    if ( r == 0 )
    {
        T->lchild = NULL;
        T->rchild = NULL;
        free(T->recYv);
        free(T->recYu);
        T->recYv = NULL;
        T->recYu = NULL;
        // add decoding of repetition code here ; give T->chat
        for (int i = 0; i < N; i++)
        {
            Ysum += ptr[i];
        }        
        temp = sign(Ysum);
        if (temp == 0 || temp == -1)  
        {
            for (int i = 0; i < N; i++)
            {
                T->chat[i] = 1;
             //   T->chathat->values[i] = 1;
            }
        }else 
        {
            for (int i = 0; i < N; i++)
            {
                T->chat[i] = 0;
             //   T->chathat->values[i] = 0;
            }
        }
        // finish decoding of repetition code here ; give T->chat  
    }else if ( r == (m-1) )
    {
        T->lchild = NULL;
        T->rchild = NULL;
        free(T->recYv);
        free(T->recYu);
        T->recYv = NULL;
        T->recYu = NULL;
        // add decoding SPC code here
        for (int i = 0; i < N; i++)
        {
            T->chat[i] = ptr[i] < 0;
            temp1 += (T->chat[i]);
           // T->chathat->values[i] = ptr[i]<0;
           // temp1 += (T->chathat->values[i]); 
        }
        if ( (temp1%2) == 1)
        {
            smallestV = fabs(ptr[0]);
            smallestInd = 0;
            for (int i = 0; i < N; i++)
            {
                if (fabs(ptr[i]) < smallestV) {
                    smallestV = fabs(ptr[i]);
                    smallestInd = i;
                }
            }
            T->chat[smallestInd] ^= 0X01;
            //T->chathat->values[smallestInd] ^= 0X01;
        }
        // end decoding SPC code here
    }else
    {
        // add the approx. function of LR recursion; BOTH LEFT AND RIGH BRANCH
        //T->recYv = rm_calc_f(ptr,  N);
        for (uint16_t i = 0; i < pus; i++)
        {
            T->recYv[i] =  sign(ptr[2*i]*ptr[2*i+1]) * fmin(fabs(ptr[2*i]),fabs(ptr[2*i+1]));
        }
        T->lchild = createTree(  T->recYv, (r-1), (m-1));
        
        //T->recYu = rm_calc_g(ptr,N,(T->lchild)->chat);
        Btree *Tv = T->lchild;
        for (uint16_t i = 0; i < pus; i++)
        {
            //T->recYu[i] = 0.5  * ( pow(-1,Tv->chat[i]) * ptr[2*i] + ptr[2*i+1] ) ;
            T->recYu[i] = 0.5  * ( (1-2*(Tv->chat[i])) * ptr[2*i] + ptr[2*i+1] ) ;
        }
        T->rchild = createTree( T->recYu, r, (m-1) );
        // add determination of the codeword here i.e. step 2c; give T->chat
        // T->chat = combsubcodes(T->lchild,T->rchild,N/2);
        Btree *Tu = T->rchild;
        for (int i = 0; i < pus; i++)
        {
            T->chat[2*i] = (Tv->chat[i])^(Tu->chat[i]);
            T->chat[2*i+1] = Tu->chat[i];   
        }
    }
    // step 3 give T->chat; NO NEED ???? Answer is no.

    return T;
    
}


double* rm_calc_f(double *recY, uint16_t n)
{
    uint16_t pus = n/2;
    double *yleft = (double*) malloc(sizeof(double)*pus);///////////////////////REMEMBER TO FREE!!!!!!!!!

    for (uint16_t i = 0; i < pus; i++)
    {
        *(yleft+i) = (double)  sign(recY[2*i]*recY[2*i+1]) * fmin(fabs(recY[2*i]),fabs(recY[2*i+1]));
    }
    
    return yleft; 
}

double* rm_calc_g(double *recY, uint16_t n, uint8_t *a1hat)
{
    uint16_t pus = n/2;
    double *yright = (double*) malloc(sizeof(double)*pus);///////////////////////REMEMBER TO FREE!!!!!!!!!
    for (uint16_t i = 0; i < pus; i++)
    {
        *(yright+i) = ( *(a1hat+i) * recY[2*i]  + recY[2*i+1] )  * 0.5;
    }
    return yright;
}
uint8_t *combsubcodes (Btree *Tv, Btree *Tu, int N)
{
    uint8_t * codeword = malloc(sizeof(uint8_t)*N) ;///////////////////////REMEMBER TO FREE!!!!!!!!!
    for (int i = 0; i < N/2; i++)
    {
        codeword[2*i] = (Tv->chat[i])^(Tu->chat[i]);
        codeword[2*i+1] = Tu->chat[i];   
    }

}
void preorder(Btree *T)
{
     if ((T)!=NULL)
    {
        uint8_t r = (T)->r;
        uint8_t m = (T)->m;
        double * recYv = (T->recYv);
        double * recYu = (T->recYu);

        printf("R(%d,%d): \n", r, m);
        printf("Received Y: ");
        for (size_t i = 0; i < (1<<(m-1)); i++)
        {
            printf("%5.1f ",*(recYv++));
        }
        for (size_t i = 0; i < (1<<(m-1)); i++)
        {
            printf("%5.1f ",*(recYu++));
        }

        printf("\n");
        preorder(((T)->lchild));
        preorder(((T)->rchild));        
    }
}
void travBTree(Btree *T, FILE *fpC, FILE *fpY, unsigned int pos)//preorder
{
    uint8_t r,m;
    if(T != NULL)
    {
        r = (T)->r;
        m = (T)->m;
        double * recYv = (T->recYv);
        double * recYu = (T->recYu);

//        fprintf(fp,"R(%d, %d), chat=%d",r,m,T->chat[0]);
        fprintf(fpC,"%d %d %d",pos,r,m);
        for (unsigned int i = 0; i < (1<<(m)); i++)
        {
            fprintf(fpC," %d",T->chat[i]);
        }
        fprintf(fpY,"%d %d %d",pos,r,m);
        if ( (recYv != NULL) && (recYu != NULL) )
        {
//            fprintf(fp, ". Yv=%5.3f",*(recYv++));
            for (size_t i = 0; i < (1<<(m-1)); i++)
            {
                fprintf(fpY, " %5.5f",*(recYv++));
            }
//            fprintf(fp, ". Yu=%5.3f",*(recYu++));
//            fprintf(fp, " %5.3f",*(recYu++));
            for (size_t i = 0; i < (1<<(m-1)); i++)
            {
                fprintf(fpY, " %5.5f ",*(recYu++));
            }

        }
        fprintf(fpC,"\n");
        fflush(fpC);
        fprintf(fpY,"\n");
        fflush(fpY);
        if ((T->lchild != NULL) && T->rchild != NULL)
        {
            pos = pos<<1;
            travBTree(((T)->lchild),fpC, fpY, pos);
            pos = pos+1;        
            travBTree(((T)->rchild),fpC, fpY, pos);
        }

    }
}

void destroyTree(Btree *T)
{
    Btree *pl,*pr;
    if ( T == NULL)
    {
        return ;
    }else
    {
        free(T->chat);
        T->chat = NULL;
        free(T->recYv);
        T->recYv = NULL;
        free(T->recYu);
        T->recYu = NULL;
        pl = T->lchild;
        pr = T->rchild;
        T->lchild = NULL;
        T->rchild = NULL;
        free(T);
        T = NULL;
        destroyTree(pl);
        destroyTree(pr);
    }
    

}