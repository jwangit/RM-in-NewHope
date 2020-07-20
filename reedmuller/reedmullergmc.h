//
//  reedmullergmc.h
//
//  Created by Jiabo Wang (Mar) on 11/03/20.
//  Copyright © 2020 Wang, Jiabo (Mar). All rights reserved.
//
#ifndef REEDMULLERGMC_H
#define REEDMULLERGMC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef struct treeNode
{
    
    uint8_t r, m;
    double *recYv;
    double *recYu;
    int8_t *chat;
//    int8_t *mhat;
    struct treeNode *lchild, *rchild;
}Btree;

Btree *createTree(double *ptr, int8_t r, int8_t m);
void travBTree(Btree *T, FILE *fpC, FILE *fpY, unsigned int pos);
void preorder(Btree *T);
void destroyTree(Btree *T);
Btree *softDecSimp(double *ptr, int8_t r, int8_t m);

int sign(double x);
double* rm_calc_f(double *recY, uint16_t n);
double* rm_calc_g(double *recY, uint16_t n, uint8_t *a1hat);
uint8_t *combsubcodes (Btree *Tv, Btree *Tu, int N);

#endif