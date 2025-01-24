// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2010 - 2024 The Minotaur Team.
// 

#include "MinotaurConfig.h"
#include "Chol.h"

using namespace Minotaur;

#ifdef F77_FUNC

extern "C"
{
  void F77_FUNC(dpotrf, DPOTRF )(char *uplo, int *n, double *a, int *lda, 
                                 int *info );
}


CholCalculator::CholCalculator()
  :n_(0),
   A_(0),
   abstol_(1e-6)
{
}

#endif

