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
          std::vector<double> & p_const, double & n_const, double sqrtK);

      /// Destroy
      ~Monomial() {};

      double eval(const double *x) const;
      void evalGradient(const double *x, double *grad_f) const;
      // UInt getNumNzInHess();
      void getHessVarPairs( VarPairIntMap & vp_inds);
      void evalHessian(const double mult, const double *x, 
          VarPairIntMap &vp_inds, double *values);
      FunctionType getType();
      void write(std::ostream &out) const;

    private:
      double coeff_;
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
