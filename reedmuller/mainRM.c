#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "reedmuller.h"

int main()
{
    setbuf(stdout,NULL);
    Btree *T=NULL;
    uint8_t par_r = 4;
    uint8_t par_m = 6;
    uint8_t *ptr = malloc(sizeof(uint8_t)*pow(2,par_m));
    printf("REED MULLER CODES RM(%u,%u) \n", par_r,par_m);
    printf("received Y is ");

    for (size_t i = 0; i < (int)pow(2,par_m); i++)
    {
        if ((i&0x01) == 0)
        {
            *(ptr+i) = 0;
        }else
        {
            *(ptr+i) = 1;
        }
        printf("%u ",*(ptr+i));
    }
    printf("\n");
    T = createTree(ptr, par_r,  par_m);
    preorder(T);
    destroyTree(T);
    return 0;
}