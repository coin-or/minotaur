/*
 * CDlogdet.c
 *
 *  Created on: Dec 10, 2013
 *      Author: Hongbo Dong
 *
 * This file implements a primal barrier coordinate descent algorithm to solve the following optimization problem
 *  min   f(d)
 *  s.t.  Q + diag(d) PSD.
 *    where f(d) is a convex nonsmooth function such that f(d) = \sum_{i=1}^n g_i(d_i), and 
 *          g_i(x) = wneg(i)*x, if x<=0,
 *                 = wpos(i)*x, if x>=0;
 *                 0 <= wneg(i) <= wpos(i)
 */

#include "CDlogdet.h"

void datacheck(int n, double* Q, double* wneg, double* wpos, double sigma, int maxIter, double* d, int* iter_arr, double* obj_arr, double* time_arr)
{
  PRINT("N=%d\n",n);
  PRINT("Q = \n");
  int i,j;
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      PRINT("%+3.3f ", Q[(j-1)*n+(i-1)]);
    }
    PRINT("\n");
  }
  PRINT("wneg = \n");
  for (i=0; i<n; i++)
  {
    PRINT("%+6.2f ", wneg[i]);
  }
  PRINT("\n");
  PRINT("wpos = \n");
  for (i=0; i<n; i++)
  {
    PRINT("%+6.2f ", wpos[i]);
  }
  PRINT("\n");
  PRINT("d = \n");
  for (i=0; i<n; i++)
  {
    PRINT("%+6.2f ", d[i]);
  }
  PRINT("\n");
}

void printMat(double* mat, int n)
{
  int i, j;
  for (i=1; i<=n; i++)
  {
    for (j=1; j<=n; j++)
    {
      PRINT("%+6.3f ", mat[(j-1)*n+(i-1)]);
    }
    PRINT("\n");
  }
  PRINT("\n");
}


void printVec(double* vec, int n)
{
  int i;
  for (i=0; i<n; i++)
  {
    PRINT("%+6.3f ", vec[i]);
  }
  PRINT("\n");
}

/* Q:        problem data, 1-dim array of length n*n
 * wneg:     slope of negative part, 1-dim array of length n
 * wpos:     slope of positive part, 1-dim array of length n
 * sigma:    initial penalty parameter
 * maxIter:  max number of iterations
 * d:        initial feasible vector d, 1-dim array of length n
 * iter_arr, obj_arr and time_arr are output variables, CURRENTLY NOT USED!!! */
int CDlogdet_nonsmooth(int n, double* Q, double* wneg, double* wpos, double sigma,
    int maxIter, double* d, double* Vinit,
    int* iter_arr, double* obj_arr, double* time_arr)
{
  int         iter         = 0;
  int         k            = 0;
  double      Vkk          = 0;
  double      dk           = 0;
  double      dchange      = 0;
  clock_t     iterTimer    = 0;
  double      GRAD_TOL     = 3E-2;
  double      MIN_TAU      = 1E-5;
  double      TERM_PROG    = 1E-4;
  double      relgrad      = 1E17;
  double      tmpScalar1   = 0;
  double      VERYSMALLNUM = 1E-9;
  double      obj          = 0;
  double      prevobj      = 1E17;
  double      nrmW         = 0;
  double      TAU_UPDATE   = 0.8;
  int         nsq          = n*n;
  vDSP_Length tmpPosition  = 0;

  double* V = NULL;
  double* vk = NULL;
  double* tmpVec1 = NULL;
  double* tmpVec2 = NULL;
  double* tmpVec3 = NULL;
  double* tmpVec4 = NULL;
  double* tmpVec5 = NULL;
  double* tmpVec6 = NULL;
  double* subg = NULL;

  V       = (double*) malloc(nsq*sizeof(double));
  vk      = (double*) malloc(n*sizeof(double));
  subg    = (double*) malloc(n*sizeof(double));
  tmpVec1 = (double*) malloc(n*sizeof(double));
  tmpVec2 = (double*) malloc(n*sizeof(double));
  tmpVec3 = (double*) malloc(n*sizeof(double));
  tmpVec4 = (double*) malloc(n*sizeof(double));
  tmpVec5 = (double*) malloc(n*sizeof(double));
  tmpVec6 = (double*) malloc(n*sizeof(double));

  char UPLO = 'L';
  int LDA = n;
  int INFO = 0;

  if (PRINTLEVEL)
  {
    PRINT("Entering CDlogdet_nonsmooth: n=%d, sigma=%f, maxIter=%d\n",n,sigma,maxIter);
  }

  iterTimer = clock();

  /*Compute V = inv(Q+d), only stores the lower triangular part*/
  if (NULL==Vinit)
  {
    cblas_dcopy(nsq, Q, 1, V, 1);
    cblas_daxpy(n, 1, d, 1, V, n+1);

    UPLO = 'L';
    LDA = n;
    INFO = 0;
    dpotrf_(&UPLO, &n, V, &LDA, &INFO);
    dpotri_(&UPLO, &n, V, &LDA, &INFO);
  }
  else
  {
    cblas_dcopy(nsq, Vinit, 1, V, 1);
  }

  //printMat(V,n);
  //printMat(Vinit,n);

  nrmW = MAX(cblas_dnrm2(n, wneg, 1), cblas_dnrm2(n, wpos, 1));
  vDSP_vmulD(wneg,1,d,1,tmpVec1,1,n);
  vDSP_vmulD(wpos,1,d,1,tmpVec2,1,n);
  vDSP_vmaxD(tmpVec1, 1, tmpVec2, 1, tmpVec3, 1, n);
  vDSP_sveD(tmpVec3,1,&prevobj,n);
  while(iter<maxIter)
  {
    /*Compute sub-gradient*/
    /*tmpVec1 <= wneg*/
    cblas_dcopy(n, wneg, 1, tmpVec1, 1);


    /*tmpVec1 <= wneg-sigma*diag(V)*/
    cblas_daxpy(n, -sigma, V, n+1, tmpVec1,1);

    /*tmpVec2 <= wpos*/
    cblas_dcopy(n, wpos, 1, tmpVec2, 1);
    /*tmpVec2 <= wpos-sigma*diag(V)*/
    cblas_daxpy(n, -sigma, V, n+1, tmpVec2,1);

    /*tmpVec3 <= max(0,wneg-sigma*diag(V)) */
    tmpScalar1 = 0;
    vDSP_vthresD(tmpVec1,1,&tmpScalar1,tmpVec3,1,n);

    /*tmpVec4 <= -min(0,wpos-sigma*diag(V))*/
    vDSP_vnegD(tmpVec2,1,tmpVec4,1,n);
    tmpScalar1 = 0;
    vDSP_vthresD(tmpVec4,1,&tmpScalar1,tmpVec4,1,n);

    /*tmpVec3 <= max(0,wneg-sigma*diag(V)) + min(0,wpos-sigma*diag(V))*/
    cblas_daxpy(n,-1,tmpVec4,1,tmpVec3,1);

    /*If d<0, use wneg-sigma*diag(V)*/
    /*tmpVec4_i = 1 <==> d_i < -VERYSMALLNUM */
    /*tmpVec4 <= (-d)*/
    vDSP_vnegD(d,1,tmpVec4,1,n);
    tmpScalar1 = 1;
    vDSP_vlimD(tmpVec4,1, &VERYSMALLNUM, &tmpScalar1,tmpVec4,1,n);
    tmpScalar1 = 1;
    vDSP_vsaddD(tmpVec4,1,&tmpScalar1, tmpVec4, 1, n);
    tmpScalar1 = 2;
    vDSP_vsdivD(tmpVec4,1,&tmpScalar1, tmpVec4, 1, n);

    /*tmpVec5[i] = 1 <==> d_i > VERYSMALLNUM*/
    cblas_dcopy(n,d,1,tmpVec5,1);
    tmpScalar1 = 1;
    vDSP_vlimD(tmpVec5, 1, &VERYSMALLNUM, &tmpScalar1, tmpVec5, 1, n);
    tmpScalar1 = 1;
    vDSP_vsaddD(tmpVec5, 1, &tmpScalar1, tmpVec5, 1, n);
    tmpScalar1 = 2;
    vDSP_vsdivD(tmpVec5,1,&tmpScalar1, tmpVec5, 1, n);

    /*tmpVec6[i] = 1 <==> abs(d_i) < VERYSMALLNUM*/
    vDSP_vaddD(tmpVec4, 1, tmpVec5, 1, tmpVec6, 1, n);
    tmpScalar1 = -1;
    vDSP_vsaddD(tmpVec6,1, &tmpScalar1, tmpVec6, 1, n);
    vDSP_vnegD(tmpVec6,1,tmpVec6,1,n);

    /*Multiply, Multiply, Multiply, then add */
    vDSP_vmmaD(tmpVec1, 1, tmpVec4, 1, tmpVec2, 1, tmpVec5, 1, subg,1, n);

    vDSP_vmaD(tmpVec3, 1, tmpVec6, 1, subg, 1, subg, 1, n);


    /*Choose the index with largest abs(subg)*/
    vDSP_maxmgviD(subg,1,&tmpScalar1,&tmpPosition,n);

    /*k is the selected index*/
    k = (int) tmpPosition;
    Vkk = V[k*n+k];
    dk  = d[k];
    if (1-dk*Vkk<=0 || (sigma*Vkk/(1-dk*Vkk)>wpos[k]) )
    {
      dchange = sigma/wpos[k] - 1/Vkk;
    }
    else if(sigma*Vkk/(1-dk*Vkk)<wneg[k])
    {
      dchange = sigma/wneg[k] - 1/Vkk;
    }
    else
    {
      dchange = -dk;
    }
    d[k] += dchange;

    cblas_dcopy(k+1, &V[k], n, vk, 1);
    cblas_dcopy(n-k-1, &V[k*n+k+1], 1, &vk[k+1], 1);

    tmpScalar1 = -(dchange/(1+dchange*Vkk));
    cblas_dsyr(CblasColMajor, CblasLower, n, tmpScalar1, vk, 1, V, n);

    iter ++;

    relgrad = cblas_dnrm2(n,subg,1)/nrmW;

    vDSP_vmulD(wneg,1,d,1,tmpVec1,1,n);
    vDSP_vmulD(wpos,1,d,1,tmpVec2,1,n);
    vDSP_vmaxD(tmpVec1, 1, tmpVec2, 1, tmpVec3, 1, n);
    vDSP_sveD(tmpVec3,1,&obj,n);

    if (relgrad<GRAD_TOL)
    {
      sigma = MAX(MIN_TAU, sigma*TAU_UPDATE);
      if (PRINTLEVEL)
      {
        PRINT("Iter = %5d, sig=%1.2e(#) , obj = %1.5e, relgrad = %1.3f\n",
            iter, sigma, obj, relgrad);
      }
    }
    else if (iter==1 || iter%n==0)
    {
      if (PRINTLEVEL)
      {
        PRINT("Iter = %5d, sig=%1.2e    , obj = %1.5e, relgrad = %1.3f, t=%0.3f\n",
            iter, sigma, obj, relgrad,((double)(clock()-iterTimer))/CLOCKS_PER_SEC);
      }
    }

    if (iter%n==0)
    {
      if (obj<prevobj && ABS(prevobj-obj)<TERM_PROG*ABS(obj))
      {
        if(PRINTLEVEL)
        {
          PRINT("Terminate due to small progress.\n");
        }
        break;
      }
      else
      {
        prevobj = obj;
      }
    }
  }

  free(V);
  free(vk);
  free(tmpVec1);
  free(tmpVec2);
  free(tmpVec3);
  free(tmpVec4);
  free(tmpVec5);
  free(tmpVec6);
  free(subg);
  return 0;
}

