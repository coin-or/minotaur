/*
 * CDlogdet.h
 *
 *  Created on: Dec 12, 2013
 *      Author: hdong
 */

#ifndef CDLOGDET_H_
#define CDLOGDET_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

//#include "mex.h"
//#define PRINT (mexPrintf)
#define PRINT printf
#include </System/Library/Frameworks/Accelerate.framework/Headers/Accelerate.h>
#include </System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers/clapack.h>
#include </System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers/cblas.h>
#include </System/Library/Frameworks/Accelerate.framework/Frameworks/vecLib.framework/Headers/vDSP.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define ABS(x) (((x) > (0)) ? (x) : (-x))

#ifdef VERBOSEON
    #define PRINTLEVEL 1
#else
    #define PRINTLEVEL 0
#endif

void datacheck(int, double*, double*, double*, double, int, double*, int*, double*, double*);
void printMat(double* mat, int n);
int CDlogdet_nonsmooth(int, double*, double*, double*, double, int, double*, int*, double*, double*);

/*int CDlogdet_fast(int, double*, double*, double, int, unsigned int*, double*);
int CDlogdet_ff(int, double*, double*, double, int, double*, int*, double*, double*);
int CDlogdet(int, double **, double *,  double, int);

int compWrapper(int n, double** Qori, double* w, double tau, int maxIter);*/

#endif /* CDLOGDET_H_ */
