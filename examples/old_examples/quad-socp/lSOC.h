// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 -- The MINOTAUR Team
// 

#ifndef MINOTAURLSOCFUNCTION_H
#define MINOTAURLSOCFUNCTION_H

#include <vector>
#include <boost/shared_ptr.hpp>

#include "Function.h"
#include "Types.h"
#include "Variable.h"

namespace Minotaur {

  // /**
  // \sqrt(\sum l_i^2 + sqrtK_) \leq l_0 + K_
  // */
  class LSOC : public Function {

    public:
      /// Default constructor
      LSOC();

      LSOC(std::vector<LinearFunctionPtr> & p_terms, LinearFunctionPtr n_term, 
          std::vector<double> & p_const, double & n_const, double sqrtK);

      /// Destroy
      ~LSOC() {};

      double eval(const double *x) const;
      void evalGradient(const double *x, double *grad_f) const;
      // UInt getNumNzInHess();
      void getHessVarPairs( VarPairIntMap & vp_inds);
      void evalHessian(const double mult, const double *x, 
          VarPairIntMap &vp_inds, double *values);
      FunctionType getType();
      void write(std::ostream &out) const;

    private:
      LinearFunctionPtr l0_;

      std::vector< LinearFunctionPtr > sqrtTerms_;
      std::vector< double > sqrtKs_;

      double sqrtK_;

      double K_;

      double sqrtVal_;
  };
  typedef boost::shared_ptr<LSOC> LSOCPtr;
}
#endif

