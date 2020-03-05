//
//  reedmuller.h
//  PolarCodes
//
//  Created by Nicola De Franceschi on 03/10/13.
//  Copyright (c) 2013 Nicola De Franceschi. All rights reserved.
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
void preorder(Btree *T);
void destroyTree(Btree *T);

int sign(double x);
double* rm_calc_f(double *recY, uint16_t n);
double* rm_calc_g(double *recY, uint16_t n, uint8_t *a1hat);
uint8_t *combsubcodes (Btree *Tv, Btree *Tu, int N);

#endif