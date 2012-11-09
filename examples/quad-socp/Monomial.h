// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 -- The MINOTAUR Team
// 

#ifndef MINOTAURMONOMIALFUNCTION_H
#define MINOTAURMONOMIALFUNCTION_H

#include <vector>
#include <boost/shared_ptr.hpp>

#include "Variable.h"
#include "Types.h"

namespace Minotaur {

  // /**
  // \sqrt(\sum l_i^2 + sqrtK_) \leq l_0 + K_
  // */
  class Monomial : public Function {

    public:
      /// Default constructor
      Monomial();

      Monomial(std::vector<LinearFunctionPtr> & p_terms, LinearFunctionPtr n_term, 
          std::vector<Double> & p_const, Double & n_const, Double sqrtK);

      /// Destroy
      ~Monomial() {};

      Double eval(const Double *x) const;
      void evalGradient(const Double *x, Double *grad_f) const;
      // UInt getNumNzInHess();
      void getHessVarPairs( VarPairIntMap & vp_inds);
      void evalHessian(const Double mult, const Double *x, 
          VarPairIntMap &vp_inds, Double *values);
      FunctionType getType();
      void write(std::ostream &out) const;

    private:
      Double coeff_;
      std::vector<VariablePtr> vars_;
      std::vector<UInt> powers_;
  };
  typedef boost::shared_ptr<Monomial> MonomialPtr;
  typedef boost::shared_ptr<const Monomial> ConstMonomialPtr;
}
#endif

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
