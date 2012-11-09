// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 -- The MINOTAUR Team
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
          std::vector<Double> & p_const, Double & n_const, Double sqrtK);

      /// Destroy
      ~LSOC() {};

      Double eval(const Double *x) const;
      void evalGradient(const Double *x, Double *grad_f) const;
      // UInt getNumNzInHess();
      void getHessVarPairs( VarPairIntMap & vp_inds);
      void evalHessian(const Double mult, const Double *x, 
          VarPairIntMap &vp_inds, Double *values);
      FunctionType getType();
      void write(std::ostream &out) const;

    private:
      LinearFunctionPtr l0_;

      std::vector< LinearFunctionPtr > sqrtTerms_;
      std::vector< Double > sqrtKs_;

      Double sqrtK_;

      Double K_;

      Double sqrtVal_;
  };
  typedef boost::shared_ptr<LSOC> LSOCPtr;
}
#endif

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
