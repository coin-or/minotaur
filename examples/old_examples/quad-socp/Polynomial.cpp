//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/** 
 This is example driver code to implement polynomial class
*/

// #include <cstdlib>
// #include <iostream>
// 
// #include "AMPLInterface.h"
// #include "Eigen.h"
// #include "Problem.h"
// 

#include <iostream>
#include <fstream>
#include <sstream>

#include "MinotaurConfig.h"

#include "Polynomial.h"

using namespace Minotaur;

Polynomial::Polynomial()
  : c_(0)
{
  starts_.push_back(0);
  //coeffs_.clear(); // size nTerms_
  //vars_.clear(); // size nzs_
  //powers_.clear(); // size nzs_
  //starts_.clear(); // size nzs_+1
}


Polynomial::Polynomial(ConstVariablePtr vPtr)
  : c_(0)
{
  coeffs_.clear(); // size nTerms_
  vars_.clear(); // size nzs_
  powers_.clear(); // size nzs_
  starts_.clear(); // size nzs_+1

  coeffs_.push_back(1.0);
  vars_.push_back(vPtr);
  powers_.push_back(1);
  starts_.push_back(0);
  starts_.push_back(1);
}


Polynomial::Polynomial(const std::vector<double> & coeffs, 
    const std::vector<UInt > & powers, 
    const std::vector<ConstVariablePtr> & vars, 
    const std::vector<UInt > & starts,
    const double & c)
{
  coeffs_.insert(coeffs_.begin(), coeffs.begin(), coeffs.end());
  powers_.insert(powers_.begin(), powers.begin(), powers.end());
  vars_.insert(vars_.begin(), vars.begin(), vars.end());
  starts_.insert(starts_.begin(), starts.begin(), starts.end());
  c_ = c;
}


Polynomial::~Polynomial()
{
  coeffs_.clear(); // size nTerms_
  vars_.clear(); // size nzs_
  powers_.clear(); // size nzs_
  starts_.clear(); // size nzs_+1
}


PolynomialPtr Polynomial::clone() const
{
  PolynomialPtr p = (PolynomialPtr) new Polynomial(coeffs_, powers_, vars_, starts_, c_);
  return p;
}


UInt Polynomial::getNumTerms() const
{
  if (fabs(c_)>1e-8) {
    return coeffs_.size() + 1;
  } else {
    return coeffs_.size();
  }
}

void Polynomial::raiseTo(const double p)
{
  UInt n = getNumTerms();
  if (n > 1) {
    assert (!"can't raiseto!");
  } else if (n > 0) {
    coeffs_[0] = pow(coeffs_[0], p);
    for (UInt j=starts_[0]; j<starts_[1]; j++) {
      powers_[j] *= p;
    }
  }
}

void Polynomial::createVarSet()
{
  std::string s;
  UInt i=0;
  for (i=0; i<vars_.size(); i++) {
    var_set_.insert(vars_[i]);
  }

  for (VarSetConstIterator iter=var_set_.begin(); iter!=var_set_.end(); iter++) {
    s = (*iter)->getName();
    var_map_.insert(std::make_pair(s, (*iter)));
  }
}


void Polynomial::factorize()
{
  //std::cout << "ring r = 0, (";
  UInt i=0;
  int numFactors;
  LinearFunctionPtr lf;
  UInt power = 0;
  double lf_const;
  //for (VarSetConstIterator iter=var_set_.begin(); iter!=var_set_.end(); iter++, i++) {
  //  std::cout << (*iter)->getName() << ", ";
  //}
  //std::cout << "d), dp;" << std::endl;
  //std::cout << "matrix m=factorize( 0 ";
  //write(std::cout);
  //std::cout << ",1);" << std::endl;

  
  std::ofstream myfile;
  myfile.open ("foo.in");

  myfile << "ring r = 0, (";
  for (VarSetConstIterator iter=var_set_.begin(); iter!=var_set_.end(); iter++, i++) {
    myfile << (*iter)->getName() << ", ";
  }
  myfile << "d), dp;" << std::endl;
  myfile << "poly p = 0 ";
  write(myfile);
  myfile << ";" << std::endl;


  myfile << "// output file" << std::endl;
  myfile << "link li=\":w foo.out\";" << std::endl;

  myfile << std::endl;
  myfile << "// factorize" << std::endl;
  myfile << "list l=factorize(p);" << std::endl;

  myfile << std::endl;
  myfile << "// print length" << std::endl;
  myfile << "intvec v=l[2];" << std::endl;
  myfile << "int d=size(v);" << std::endl;
  myfile << "fprintf (li, \"%s\", eval(d));" << std::endl;

  myfile << std::endl;
  myfile << "// print output using a loop" << std::endl;
  myfile << "matrix m[d][1]=l[1];" << std::endl;
  myfile << "for (int i=1; i<=d; i=i+1)" << std::endl;
  myfile << "{" << std::endl;
  myfile << "  fprintf (li, \"%s  %s\", m[1][i], v[i]);" << std::endl;
  myfile << "}" << std::endl;
  myfile << "exit;" << std::endl;
  myfile.close();

  assert(system ("/sandbox/mahajan/singularity/Singular/3-1-1/x86_64-Linux/Singular -q foo.in")==0);

  // read the factors
  std::string line;
  std::ifstream outfile ("foo.out");
  outfile >> numFactors;
  std::cout << "no. of factors = " << numFactors << std::endl;
  outfile >> f_coeff_;
  outfile >> line;
  std::cout << "const factor = " << f_coeff_ << std::endl;
  outfile >> line;
  while (!outfile.eof()) {
    lf = getLf_(line, lf_const);

    power = 0;
    outfile >> line;
    std::stringstream myStream(line);
    myStream >> power;
    for (UInt i=0; i<power; i++) {
      lfs_.push_back(lf);
      lfs_cb_.push_back(lf_const);
    }

    outfile >> line;
  }
  outfile.close();
  for (UInt i=0; i< lfs_.size(); i++) {
    lfs_[i]->write(std::cout);
    std::cout << "+ " << lfs_cb_[i] << std::endl;
  }
}


LinearFunctionPtr Polynomial::getLf_(std::string line, double & cb)
{
  LinearFunctionPtr lf;
  int pos = 0;
  int len = 0;
  std::string varName;
  double varCoeff;
  cb = 0;

  // find the first multiplier/variable in string.
  lf = (LinearFunctionPtr) new LinearFunction();
  while (pos < line.size()) {
    getNextTerm_(line, pos, varName, varCoeff, len);
    if (varName.size() == 0) {
      cb = varCoeff;
    } else {
      lf->addTerm(var_map_[varName], varCoeff);
    }
    pos += len;
  }
  return lf;
}

void Polynomial::getNextTerm_(std::string line, int pos, std::string & varName, double & varCoeff, int & len)
{
  varName = "";
  varCoeff = 0.0;
  len = 0;
  // get the first char
  varCoeff = 1.0;
  double mult = 1.0;
  int pos1, pos2;
  std::string tstr;

  pos2 = pos;
  if (line.at(pos) == '-') {
    mult = -1.0;
    varCoeff = -1.0;
    pos++;
  } else if (line.at(pos) == '+') {
    mult = 1.0;
    pos++;
  }
  pos1 = pos;
  if (line.at(pos) >= '0' && line.at(pos) <= '9') {
    // we are looking at a constant term
    while (pos < line.size()) {
      if (line.at(pos) == '+' || line.at(pos) == '-' || line.at(pos) == '/' 
          || line.at(pos) == '*') {
        break;
      }
      pos++;
    }
    tstr = line.substr(pos1, pos-pos1);
    std::stringstream myStream(tstr);
    myStream >> varCoeff;
    varCoeff *= mult;
    pos1 = pos;
  }
  if (pos<line.size() && (line.at(pos) < '0' || line.at(pos) > '9')) {
    // we are looking at a variable name
    if (line.at(pos) == '*') {
      pos++;
      pos1++;
    }
    while (pos < line.size()) {
      if (line.at(pos) == '+' || line.at(pos) == '-' || line.at(pos) == '/' 
          || line.at(pos) == '*') {
        break;
      }
      pos++;
    }
    varName = line.substr(pos1, pos-pos1);
  }
  len = pos - pos2;
}


void Polynomial::write(std::ostream &out) const
{
  for (UInt i=0; i<coeffs_.size(); i++) {
    if (coeffs_[i] >= 0.0) {
      out << " + ";
    } else {
      out << " - ";
    } 
    out << fabs(coeffs_[i]);
    for (UInt j=starts_[i]; j<starts_[i+1]; j++) {
      out << "*";
      out << "x" << vars_[j]->getId();
      out << "^" << powers_[j];
    }
  }
}

void Polynomial::operator*=(ConstPolynomialPtr p2)
{
  UInt n1 = getNumTerms();
  UInt n2 = p2->getNumTerms();
  UInt j, j2;

  if (n1 > 1 || n2 > 1) {
    assert (!"can't use *= operator !");
  }
  coeffs_[0] *= p2->coeffs_[0];
  for (j2=p2->starts_[0]; j2<p2->starts_[1]; j2++) {
    for (j=starts_[0]; j<starts_[1]; j++) {
      if (vars_[j] == p2->vars_[j2]) {
        powers_[j] += p2->powers_[j2];
        break;
      }
    }
    if (j == starts_[1]) {
      starts_[1] += 1;
      vars_.push_back(p2->vars_[j2]);
      powers_.push_back(p2->powers_[j2]);
    }
  }
}

void Polynomial::operator*=(const double c)
{
  for (UInt j=0; j<coeffs_.size(); j++) {
    coeffs_[j] *= c;
  }
}

void Polynomial::operator+=(ConstPolynomialPtr p2)
{
  std::vector<UInt>::const_iterator it;
  UInt n = powers_.size();

  vars_.insert(vars_.end(), p2->varsBegin(), p2->varsEnd());
  powers_.insert(powers_.end(), p2->powersBegin(), p2->powersEnd());
  coeffs_.insert(coeffs_.end(), p2->coeffsBegin(), p2->coeffsEnd());
  for (it=p2->startsBegin()+1; it!=p2->startsEnd(); it++) {
    starts_.push_back(n+(*it));
  }
}


namespace Minotaur {
PolynomialPtr operator-(ConstPolynomialPtr p1, 
     ConstPolynomialPtr p2)
{
  PolynomialPtr pf = PolynomialPtr();  //NULL
  return pf;
}


PolynomialPtr operator+(ConstPolynomialPtr p1, 
     ConstPolynomialPtr p2)
{
  PolynomialPtr pf = PolynomialPtr();  //NULL
  return pf;
}


PolynomialPtr operator*(const double c, 
    ConstPolynomialPtr p2)
{
  PolynomialPtr p = p2->clone();
  (*p) *= c;
  return p;
}


}


