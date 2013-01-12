// 
//    MINOTAUR -- It's only 1/2 bull
// 
//    (C)opyright 2009 - 2013 The MINOTAUR Team.
// 



#include "MinotaurConfig.h"
#include "LapackUT.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LapackTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(LapackTest, "LapackUT");

using namespace Minotaur;

#define WRAPPER_TEST F77_FUNC(foobar,FOOBAR)

extern "C"
{
  void WRAPPER_TEST(double *x, double *y)
  {
    *x = 100;
    *y = -100;
  }

  void F77_FUNC(dsyev,DSYEV)(char *jobz, char *uplo, Int *n,
                             double *A, Int *ldA, double *W,
                             double *WORK, Int *LWORK, Int *info);

  void F77_FUNC(dsyevr, DSYEVR)(char *jobz, char *range, char *uplo, Int *n,
      Double *a, Int *lda, Int *vl, Int *vu, Int *il, Int *iu, Double *abstol,
      Int *m, Double *w, Double **z, Int *ldz, Int *isuppz, Double *work, Int
      *lwork, Int *iwork, Int *liwork, Int *info );
}


void LapackTest::testWrapper()
{
  Double xx=0, yy=0;
  WRAPPER_TEST(&xx, &yy);
  CPPUNIT_ASSERT(xx==100);
  CPPUNIT_ASSERT(yy==-100);
}


void LapackTest::testEigenValues()
{
// Find eigen values using DSYEV of:
//  0    1
//  1    0
  char jobz = 'N';  // N for eigen values only, V for values and vectors.
  char uplo = 'L';  // L for storing only lower triangular part of the matrix,
                    // U for upper.
  Int n = 2;        // order of matrix.
  Double *A;        // On input, the original matrix, on output ...
  Int lda = 2;      // The leading dimension of A, lda >= max(1,n)
  Double *w=0;      // output array
  Double *work;     // Work space.
  Int lwork;        // length of the array 'work'
  Int info = 0;     //  0 => successful exit
                    // -i => i-th argument has some problems
                    //  i => i off-diagonal elements of an intermediate
                    //       tridiagonal form did not converge to zero.

  A = new Double[4];
  A[0] = 0.0;
  A[1] = 1.0;
  A[2] = 1.0;
  A[3] = 0.0;

  work = new Double[1];
  work[0] = -1;
  lwork = -1;

  F77_FUNC(dsyev,DSYEV)(&jobz, &uplo, &n, A, &lda, w,
      work, &lwork, &info);
  CPPUNIT_ASSERT(info==0);

  lwork = work[0];
  delete [] work;

  work = new Double[lwork];
  w = new Double[n];
  w[0] = w[1] = 0;
  F77_FUNC(dsyev,DSYEV)(&jobz, &uplo, &n, A, &lda, w,
      work, &lwork, &info);
  CPPUNIT_ASSERT(info==0);

  CPPUNIT_ASSERT(w[0]==-1);
  CPPUNIT_ASSERT(w[1]==1);
  delete [] work;
  delete [] w;
  delete [] A;
}

void LapackTest::testEigenValues2()
{
// Find eigen values using DSYEVR of:
//  0    *
//  1    0
//  http://www.netlib.org/lapack/explore-html/dsyevr.f.html
  char jobz = 'N';  // N for eigen values only, V for values and vectors.
  char range = 'A'; // A for all eigen values only, V for values in range (vl, vu]
                    // I for il-th through iu-th eigen value.
  char uplo = 'L';  // L for storing only lower triangular part of the matrix,
                    // U for upper.
  Int n = 2;        // order of matrix.
  Double *A;        // On input, the original matrix, on output ...
  Int lda = 2;      // The leading dimension of A, lda >= max(1,n)
  Int vl=0, vu=0;   // Not used when range='A'
  Int il=0, iu=0;   // Not used when ='A'
  Double abstol = 1e-6; // The absolute error tolerance for the eigenvalues.
                        // An approximate eigenvalue is accepted as converged
                        // when it is determined to lie in an interval [a,b]
                        // of width less than or equal to
                        //         ABSTOL + EPS *   max( |a|,|b| ) ,
  Int m=0;          // number of eigen values that this routine found.
  Double *w;        // output array of eigen values.
  Double *z = NULL; // the first M columns of z
                    // contain the orthonormal eigenvectors of the matrix A
                    // corresponding to the selected eigenvalues, with the i-th
                    // column of Z holding the eigenvector associated with W(i).
                    // If JOBZ = 'N', then Z is not referenced.
                    // Note: the user must ensure that at least max(1,M) columns are
                    // supplied in the array Z; if RANGE = 'V', the exact value of M
                    // is not known in advance and an upper bound must be used.
                    // Supplying N columns is always safe.
  Int ldz = 1;      // The leading dimension of the array z.
  Int *isuppz=NULL; // The i-th eigenvector is nonzero only in elements 
                    // ISUPPZ( 2*i-1 ) through ISUPPZ( 2*i ).
  Double *work;     // Work space.
  Int lwork;        // length of the array 'work'
  Int *iwork;       // Work space.
  Int liwork;       // length of the array 'iwork'
  Int info = 0;     //  0 => successful exit
                    // -i => i-th argument has some problems
                    //  i => i off-diagonal elements of an intermediate
                    //       tridiagonal form did not converge to zero.

  A = new Double[4];
  w = new Double[2];
  work = new Double[1];
  iwork = new Int[1];
  lwork = -1;
  liwork = -1;
  w[0] = w[1] = 0;
  work[0] = 0.0;
  iwork[0] = 0;
  A[0] = 0;
  A[1] = 1;
  A[2] = 0; // This is never used by LAPACK
  A[3] = 0;

  F77_FUNC(dsyevr,DSYEVR)(&jobz, &range, &uplo, &n, A, &lda, &vl, &vu, &il, 
      &iu, &abstol, &m, w, &z, &ldz, isuppz, work, &lwork, iwork, &liwork, 
      &info);

  CPPUNIT_ASSERT(info==0);
  lwork = work[0];
  liwork = iwork[0];
  delete [] work;
  delete [] iwork;
  work = new Double[lwork];
  iwork = new Int[liwork];
   F77_FUNC(dsyevr,DSYEVR)(&jobz, &range, &uplo, &n, A, &lda, &vl, &vu, &il, 
       &iu, &abstol, &m, w, &z, &ldz, isuppz, work, &lwork, iwork, &liwork, 
       &info);
  CPPUNIT_ASSERT(info==0);
  CPPUNIT_ASSERT(w[0]==-1);
  CPPUNIT_ASSERT(w[1]==1);

// Find eigen values using DSYEVR of:
//  1    *
//  0    1
  A[0] = 1;
  A[1] = 0;
  A[2] = 0;
  A[3] = 7;
  F77_FUNC(dsyevr,DSYEVR)(&jobz, &range, &uplo, &n, A, &lda, &vl, &vu, &il, 
      &iu, &abstol, &m, w, &z, &ldz, isuppz, work, &lwork, iwork, &liwork, 
      &info);
  CPPUNIT_ASSERT(w[0]==1.0);
  CPPUNIT_ASSERT(w[1]==7.0);

// Find eigen values using DSYEVR of:
//  1    *
//  4    1
  A[0] = 1;
  A[1] = 4;
  A[2] = 4;
  A[3] = 1;
  F77_FUNC(dsyevr,DSYEVR)(&jobz, &range, &uplo, &n, A, &lda, &vl, &vu, &il, 
      &iu, &abstol, &m, w, &z, &ldz, isuppz, work, &lwork, iwork, &liwork, 
      &info);
  CPPUNIT_ASSERT(w[0]==-3.0);
  CPPUNIT_ASSERT(w[1]==5.0);

  delete [] work;
  delete [] iwork;
  delete [] w;
  delete [] A;

}

// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
