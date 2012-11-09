// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 -- The MINOTAUR Team
// 

#ifndef MINOTAURPOLYNOMIALFUNCTION_H
#define MINOTAURPOLYNOMIALFUNCTION_H

#include <vector>
#include <boost/shared_ptr.hpp>

#include "Variable.h"
#include "Function.h"
#include "Types.h"

namespace Minotaur {

  class Polynomial;
  typedef boost::shared_ptr<Polynomial> PolynomialPtr;
  typedef boost::shared_ptr<const Polynomial> ConstPolynomialPtr;
  typedef std::vector<PolynomialPtr> PolyVector;
  typedef PolyVector::iterator PolyVectorIterator;

  class Polynomial : public Function {

    public:
      /// Default constructor
      Polynomial();

      Polynomial(ConstVariablePtr vPtr);
      Polynomial(const std::vector<Double> & coeffs, 
          const std::vector<UInt > & powers, 
          const std::vector<ConstVariablePtr> & vars, 
          const std::vector<UInt > & starts,
          const Double & c);
      PolynomialPtr clone() const;

      /// Destroy
      ~Polynomial();

      Double eval(const Double *x) const {};
      void evalGradient(const Double *x, Double *grad_f) const {};
      // UInt getNumNzInHess();
      void getHessVarPairs( VarPairIntMap & vp_inds) {};
      void evalHessian(const Double mult, const Double *x, 
          VarPairIntMap &vp_inds, Double *values) {};
      FunctionType getType() {};
      void write(std::ostream &out) const;
      UInt getNumTerms() const;
      void raiseTo(const Double p);
      void factorize();
      void createVarSet();

      void setRhs(Double val) {rhs_ = val;};
      bool isNeg() {if (rhs_/f_coeff_<0) return true; else return false; };
      Double getRhs() {return fabs(rhs_/f_coeff_);};
      UInt getNumFactors() const {return lfs_.size();};
      ConstVariablePtr getAuxVar(UInt i) const {return aux_vars_[i];};
      void addAuxVar(ConstVariablePtr v) {aux_vars_.push_back(v);};

      std::vector<UInt>::const_iterator startsBegin() const {return starts_.begin();}
      std::vector<UInt>::const_iterator startsEnd() const {return starts_.end();}

      std::vector<UInt>::const_iterator powersBegin() const {return powers_.begin();}
      std::vector<UInt>::const_iterator powersEnd() const {return powers_.end();}

      std::vector<ConstVariablePtr>::const_iterator varsBegin() const {return vars_.begin();}
      std::vector<ConstVariablePtr>::const_iterator varsEnd() const {return vars_.end();}

      std::vector<Double>::const_iterator coeffsBegin() const {return coeffs_.begin();}
      std::vector<Double>::const_iterator coeffsEnd() const {return coeffs_.end();}

      std::vector<ConstLinearFunctionPtr>::const_iterator lfsBegin() const {return lfs_.begin();}
      std::vector<ConstLinearFunctionPtr>::const_iterator lfsEnd() const {return lfs_.end();}

      std::vector<Double>::const_iterator lfsCBegin() const {return lfs_cb_.begin();}
      std::vector<Double>::const_iterator lfsCEnd() const {return lfs_cb_.end();}

      void operator+=(ConstPolynomialPtr p2);
      void operator*=(const Double c);
      void operator*=(ConstPolynomialPtr p2);
      friend PolynomialPtr operator + (ConstPolynomialPtr p1, 
          ConstPolynomialPtr p2);
      friend PolynomialPtr operator - (ConstPolynomialPtr p1, 
          ConstPolynomialPtr p2);
      friend PolynomialPtr operator*(const Double c, 
          ConstPolynomialPtr p2);

    private:
      Double c_; // constant

      std::vector< Double > coeffs_; // size nTerms_
      std::vector< ConstVariablePtr > vars_; // size nzs_
      std::vector< UInt > powers_; // size nzs_
      std::vector< UInt > starts_; // size nzs_+1
      VariableSet var_set_; 
      std::map< std::string, ConstVariablePtr> var_map_;

      // after factorization
      Double f_coeff_;
      std::vector< ConstLinearFunctionPtr > lfs_;
      std::vector< Double > lfs_cb_;

      // used to replace each factor.
      std::vector<ConstVariablePtr> aux_vars_;
      // rhs
      Double rhs_;

      LinearFunctionPtr getLf_(std::string line, Double & cb);
      void getNextTerm_(std::string line, Int pos, std::string & varName, Double & varCoeff, Int & len);
  };
}
#endif

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
