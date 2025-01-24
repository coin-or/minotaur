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
      Polynomial(const std::vector<double> & coeffs, 
          const std::vector<UInt > & powers, 
          const std::vector<ConstVariablePtr> & vars, 
          const std::vector<UInt > & starts,
          const double & c);
      PolynomialPtr clone() const;

      /// Destroy
      ~Polynomial();

      double eval(const double *x) const {};
      void evalGradient(const double *x, double *grad_f) const {};
      // UInt getNumNzInHess();
      void getHessVarPairs( VarPairIntMap & vp_inds) {};
      void evalHessian(const double mult, const double *x, 
          VarPairIntMap &vp_inds, double *values) {};
      FunctionType getType() {};
      void write(std::ostream &out) const;
      UInt getNumTerms() const;
      void raiseTo(const double p);
      void factorize();
      void createVarSet();

      void setRhs(double val) {rhs_ = val;};
      bool isNeg() {if (rhs_/f_coeff_<0) return true; else return false; };
      double getRhs() {return fabs(rhs_/f_coeff_);};
      UInt getNumFactors() const {return lfs_.size();};
      ConstVariablePtr getAuxVar(UInt i) const {return aux_vars_[i];};
      void addAuxVar(ConstVariablePtr v) {aux_vars_.push_back(v);};

      std::vector<UInt>::const_iterator startsBegin() const {return starts_.begin();}
      std::vector<UInt>::const_iterator startsEnd() const {return starts_.end();}

      std::vector<UInt>::const_iterator powersBegin() const {return powers_.begin();}
      std::vector<UInt>::const_iterator powersEnd() const {return powers_.end();}

      std::vector<ConstVariablePtr>::const_iterator varsBegin() const {return vars_.begin();}
      std::vector<ConstVariablePtr>::const_iterator varsEnd() const {return vars_.end();}

      std::vector<double>::const_iterator coeffsBegin() const {return coeffs_.begin();}
      std::vector<double>::const_iterator coeffsEnd() const {return coeffs_.end();}

      std::vector<ConstLinearFunctionPtr>::const_iterator lfsBegin() const {return lfs_.begin();}
      std::vector<ConstLinearFunctionPtr>::const_iterator lfsEnd() const {return lfs_.end();}

      std::vector<double>::const_iterator lfsCBegin() const {return lfs_cb_.begin();}
      std::vector<double>::const_iterator lfsCEnd() const {return lfs_cb_.end();}

      void operator+=(ConstPolynomialPtr p2);
      void operator*=(const double c);
      void operator*=(ConstPolynomialPtr p2);
      friend PolynomialPtr operator + (ConstPolynomialPtr p1, 
          ConstPolynomialPtr p2);
      friend PolynomialPtr operator - (ConstPolynomialPtr p1, 
          ConstPolynomialPtr p2);
      friend PolynomialPtr operator*(const double c, 
          ConstPolynomialPtr p2);

    private:
      double c_; // constant

      std::vector< double > coeffs_; // size nTerms_
      std::vector< ConstVariablePtr > vars_; // size nzs_
      std::vector< UInt > powers_; // size nzs_
      std::vector< UInt > starts_; // size nzs_+1
      VariableSet var_set_; 
      std::map< std::string, ConstVariablePtr> var_map_;

      // after factorization
      double f_coeff_;
      std::vector< ConstLinearFunctionPtr > lfs_;
      std::vector< double > lfs_cb_;

      // used to replace each factor.
      std::vector<ConstVariablePtr> aux_vars_;
      // rhs
      double rhs_;

      LinearFunctionPtr getLf_(std::string line, double & cb);
      void getNextTerm_(std::string line, int pos, std::string & varName, double & varCoeff, int & len);
  };
}
#endif

