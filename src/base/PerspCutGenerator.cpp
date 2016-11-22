//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
//


/**
 * \file PerspCutGenerator.cpp 
 * \brief Declare PerspCutGenerator class. 
 * \author Meenarli Sharma, Indian Institute of Technology Bombay 
*/

#include <limits>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
using std::endl;
using std::flush;
#include <sstream>
using std::ostringstream;

#include "PerspCutGenerator.h"
#include "Function.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "Operations.h"
#include "Option.h"

# define SPEW 0

using namespace Minotaur;
const std::string PerspCutGenerator::me_ = "Perspective cut generator: ";

PerspCutGenerator::PerspCutGenerator() 
 : s_(SolutionPtr()), cp_(ConstraintPtr()), bp_(VariablePtr()), objtol_(1e-6),solAbsTol_(1e-5),
  solRelTol_(1e-5), eTol_(1e-6), lf_(LinearFunctionPtr()), rnv_(0)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}

PerspCutGenerator::PerspCutGenerator(UInt relvars, ConstSolutionPtr sol,
                                     ConstConstraintPtr c, ConstVariablePtr v)
  : s_(sol), cp_(c), bp_(v), objtol_(1e-6),solAbsTol_(1e-5),
  solRelTol_(1e-5), eTol_(1e-6), lf_(LinearFunctionPtr()), rnv_(relvars)

{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}

void PerspCutGenerator::generateCut()
{
  const double * x = s_->getPrimal();
 
  UInt binindex;
  binindex = bp_->getIndex();
 
 FunctionPtr f = cp_->getFunction();
 UInt indexvar = 0;
 ConstVariablePtr cvp;
 double sb = x[binindex];
 double vsol = 0.0;

 // Generate solution y=(x/z,1), x are continuous and z is binary variable
 double * y = new double[rnv_];
 std::fill(y, y + rnv_, 0);
 for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
   cvp = *it;
   indexvar = cvp->getIndex();
   //std::cout << cvp->getName() << " " << x[indexvar] << std::endl;
   if (indexvar != binindex) {
     vsol = x[indexvar];
     y[indexvar] = vsol/sb;  
     //std::cout << "corresponding y " << y[indexvar] << std::endl;
   } 
 } 

 y[binindex] = 1;
 gPCut(f,y);
 delete [] y;
 return;
}

void PerspCutGenerator::gPCut(FunctionPtr f, double * y)
{
  int  error = 0, errorgr = 0;
  //NonlinearFunctionPtr nlf = cp_->getNonlinearFunction();
  LinearFunctionPtr lf = cp_->getLinearFunction();
  double conseval = f->eval(y, &error);
  double * cgrad = new double[rnv_];
  std::fill(cgrad, cgrad + rnv_, 0);
  f->evalGradient(y, cgrad, &errorgr);
  lf_ = (LinearFunctionPtr) new LinearFunction();

  if (errorgr == 0 && error ==0) {
    double gradu =0.0 ;
    // Gradient of continuous variables in the constraint.
    double gradfi = 0.0;
    // Index of current variable considered.
     UInt varindex = 0;
    // Soluion of variable at vector y.
    double ysolvar = 0.0;
    ConstVariablePtr var ;
    ConstraintPtr newc;
    for (VarSetConstIterator it = f->varsBegin(); it!= f->varsEnd(); ++it) {
      var = *it; 
      varindex = var->getIndex();
      //std::cout << var->getName() << " " << y[varindex] << std::endl;
      if(varindex!=bp_->getIndex()){
        gradfi =cgrad[varindex];
        //std::cout << var->getName() << " Gradient:  " << gradfi << std::endl;

        // Add terms for each variable.
        lf_->addTerm(var, gradfi);
        // Solution value of variable.
        ysolvar = y[varindex];
        // gradient for variable u is calculated.
        gradu -= (gradfi * ysolvar); 
      } 
      //else {
        //gradu +=lf->getWeight(bp_);
      //}
    }
    // In perspective reformulated constraint coefficient of binary variable is
    // included in conseval
    // its coefficient in original constraint minus the upper bound of the constraint
    gradu-=(cp_->getUb());
    gradu+=conseval;
    // We can add binary variable to linearization, when we considered all
    // variables except the binary variable so that the linearization
    // coefficient of binary variable is calculated.
    lf_->addTerm(bp_, gradu);
    return;
  } else {
    logger_->msgStream(LogError) << me_ <<" Gradient"
     <<  " or nonlinear part of the function is not defined at the current point" 
     <<std::endl;
  }
  return;
}


//FunctionPtr PerspCutGenerator::getPFunction()
//{
  //FunctionPtr f = (FunctionPtr) new Function(lf_);
  //return f;
//}


PerspCutGenerator::~PerspCutGenerator()
{
  lf_.reset();
  cp_.reset();
  bp_.reset();
  s_.reset();
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
