#include "reedmuller.h"
#include <math.h>
Btree *createTree(double *ptr, int8_t r, int8_t m)
{
    int N = (int) pow(2,m);
    int pus =N/2;
    double Ysum = 0;
    int temp = 0;
    int temp1 = 0;
    int smallestV = 0;
    int smallestInd = 0;
    Btree *T;
    T = malloc(sizeof(Btree)+sizeof(double)*N+sizeof(uint8_t)*N); ///////////////////////REMEMBER TO FREE!!!!!!!!!
    (T) -> r = r;
    (T) -> m = m;
 
    if ( r == 0 )
    {
        T->lchild = NULL;
        T->rchild = NULL;
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
            }
        }else 
        {
            for (int i = 0; i < N; i++)
            {
                T->chat[i] = 0;
            }
        }
        // finish decoding of repetition code here ; give T->chat  
    }else if ( r == (m-1) )
    {
        T->lchild = NULL;
        T->rchild = NULL;
        T->recYv = NULL;
        T->recYu = NULL;
        // add decoding SPC code here
        for (int i = 0; i < N; i++)
        {
            T->chat[i] = ptr[i]<0;
            temp1 += (T->chat[i]); 
        }
        if ( (temp1%2) == 1)
        {
            smallestV = fabs(ptr[0]);
            smallestInd = 0;
            for (int i = 0; i < N; i++)
            {
                if (ptr[i] < smallestV) {
                    smallestV = ptr[i];
                    smallestInd = i;
                }
            }
            T->chat[smallestInd] ^= 0X01;
        }
        // end decoding SPC code here
    }else
    {
        // add the approx. function of LR recursion; BOTH LEFT AND RIGH BRANCH
        //T->recYv = rm_calc_f(ptr,  N);
        for (uint16_t i = 0; i < pus; i++)
        {
            T->recYv[i] = (double)  sign(ptr[2*i]*ptr[2*i+1]) * fmin(fabs(ptr[2*i]),fabs(ptr[2*i+1]));
        }
        T->lchild = createTree(  T->recYv, (r-1), (m-1));
        
        //T->recYu = rm_calc_g(ptr,N,(T->lchild)->chat);
        Btree *Tv = T->lchild;
        for (uint16_t i = 0; i < pus; i++)
        {
            T->recYu[i] = 0.5  * ( pow(-1,Tv->chat[i]) * ptr[2*i] + ptr[2*i+1] ) ;
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
int sign(double x)
{
	int z;
    z = (x>0) - (x<0);
	return(z);
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
    uint8_t * codeword = malloc(sizeof(uint8_t)*N) ;
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
        uint8_t * recYv = (T)->recYv;
        uint8_t * recYu = T->recYu;

        printf("R(%d,%d): \n", r, m);
        printf("Received Y: ");
        for (size_t i = 0; i < ((int)pow(2,m-1)); i++)
        {
            printf("%d ",*(recYv++));
        }
        for (size_t i = 0; i < ((int)pow(2,m-1)); i++)
        {
            printf("%d ",*(recYu++));
        }

        printf("\n");
        preorder(((T)->lchild));
        preorder(((T)->rchild));        
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
        pl = (T)->lchild;
        pr = (T)->rchild;
        (T)->lchild = NULL;
        (T)->rchild = NULL;
        free(T);
        (T) = NULL;
        destroyTree(pl);
        destroyTree(pr);
    }
    

}