//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

/** 
 This is example driver code to convert quadratic constraints into socps if
 possible and solve them using a nonlinear solver.
*/

// #include <cstdlib>
// #include <iostream>
// 
// #include "AMPLInterface.h"
// #include "Eigen.h"
// #include "Problem.h"
// 

#include "MinotaurConfig.h"

#include "AMPLInterface.h"
#include "Eigen.h"
#include "Presolver.h"
#include "Problem.h"
#include "Timer.h"
#include "lSOC.h"

#include "MinotaurDeconfig.h"
#include "IpoptEngine.h"
#include "MinotaurDeconfig.h"
#include "MinotaurConfig.h"

using namespace Minotaur;
void countEigenValues(ProblemPtr inst, UInt & numSOCs);
ProblemPtr createProblem(UInt mybit, ProblemPtr inst);

void usage()
{
  std::cout << "usage: ./quad-socp {name-of-.nl-file}\n";
}


int main(int argc, char** argv)
{
  UInt numSOCs;
  UInt nlps_solved = 0;
  UInt inf_nlps = 0;
  TimerFactory *tFactory = new Minotaur::TimerFactory();     
  Timer *timer=tFactory->GetTimer();

  timer->Start();
  double best_value = INFINITY;

  if (argc < 2) {
    usage();
    return -1;
  }
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface();
  ProblemPtr inst, newInst;
  inst = iface->readInstance(argv[1]);
  inst->calculateSize();

  if (inst->isLinear() || inst->isQuadratic()) {
    PresolverPtr presolver = (PresolverPtr) new Presolver(inst);
    presolver->standardize();
    inst->calculateSize(true);
    inst->writeSize();
    countEigenValues(inst, numSOCs);
  }

  ConstProblemSizePtr size = inst->getSize();
  if (numSOCs == size->consWithQuad && size->consWithNonlin == 0) {
    std::cout << "Can solve, numSOCs = " << numSOCs << std::endl;
  } else {
    std::cout << "Can't solve!\n";
  }

  UInt finalbit = 1 << numSOCs;
  UInt mybit = 0;
  double obj, best_obj=INFINITY;
  //double ix[] = {-1.0, -1.0};
  // 2^(numSOCs) branches.
  while (mybit < finalbit) {
    //std::cout << "my bit = " << mybit << std::endl;
    newInst = createProblem(mybit, inst);
    newInst->setQuadraticJacobian();
    newInst->setQuadraticHessian();
    //newInst->setInitialPoint(ix);
    IpoptEnginePtr ipopt_e = (IpoptEnginePtr) new IpoptEngine();
    //newInst->write(std::cout);
    ipopt_e->load(newInst);

    //solve
    ipopt_e->solve();
    nlps_solved++;

    if (ipopt_e->getStatus() == ProvenLocalOptimal) {
      //std::cout << "optimal value = " << ipopt_e->getSolutionValue();
      if (ipopt_e->getSolutionValue() < best_value) {
        best_value = ipopt_e->getSolutionValue();
        std::cout << "best value = " << best_value << std::endl;
      }
    } else {
      //std::cout << "not optimal = " << ipopt_e->getStatus() << std::endl;
      inf_nlps++;
    }
    mybit++;
  }

  std::cout << "optimal solution value = " << best_value << std::endl;
  std::cout << "NLPs solved = " << nlps_solved << std::endl;
  std::cout << "infeasible NLPs = " << inf_nlps << std::endl;
  std::cout << "time used = " << timer->Query() << std::endl;

  delete timer;
  delete tFactory;
  return 0;
}


void countEigenValues(ProblemPtr inst, UInt & numSOCs)
{
  UInt total = 0; // total number of eigen values
  UInt tpos  = 0; // total number of positive eigen values
  UInt tneg  = 0; // total number of negative eigen values
  UInt ineq  = 0; // no. of inequalities that have quadratic.
  UInt qc    = 0; // number of constraints with quadratic function.
  UInt cx    = 0; // total number <= constraints with all eigen values positive
                  //     + number >= constraints with all eigen values negative
  UInt cv    = 0; // total number <= constraints with all eigen values negative
                  //     + number >= constraints with all eigen values positive
  UInt cx1   = 0; // total number <= constraints with one e. value negative
                  //     + number >= constraints with one e. value positive
  UInt cx1q  = 0; // same condition as cx1 but also that there is no 
                  // linear function in the constraint.
  UInt cx1qc = 0; // same condition as cx1q but also that the rhs for <= is 
                  // non-positive and for >= is non-negative.
  UInt eq    = 0; // no. of equalities that have quadratic.
  UInt eqp   = 0; // no. of equalities that have all positive or 
                  // all negative e. values.
  UInt eq1   = 0; // no. of equalities that have both positive and negative 
                  // but at least one of the groups has only one member.
  UInt eq1q  = 0; // same as eq1 but the constraint has no linear terms.
  double c;
  // we will express each quadratic constraint of the form x'Ax + bx + c as
  // \sum_i (l_i x + b_i)^2 + lx + c'. 
  // We will do this transformation only if the constraint has 1 negative
  // eigen value.
  std::vector <LinearFunctionPtr> p_terms, n_terms;
  std::vector <double> p_const, n_const;
  LinearFunctionPtr lin_terms;

  ConstraintConstIterator cIter;
  ConstConstraintPtr cPtr;
  EigenCalculator *eCalc = new EigenCalculator();
  EigenPtr eigenPtr;
  UInt posi, neg, zero;

  numSOCs = 0;
  inst->getLogger()->SetMaxLevel(LogExtraInfo);
  inst->writeSize();

  for (cIter = inst->consBegin(); cIter != inst->consEnd(); cIter++) {
    cPtr = *cIter;
    if (cPtr->getFunctionType() == Quadratic || 
        cPtr->getFunctionType() == Bilinear) {
      qc++;

      // get eigen values.
      c = 0;
      eCalc->getSumOfSquares (p_terms, n_terms, p_const, n_const,
          lin_terms, c, cPtr->getQuadraticFunction(), 
          cPtr->getLinearFunction());
      neg  = n_terms.size();
      posi = p_terms.size();
      zero = cPtr->getQuadraticFunction()->getVarMap()->size() - neg - posi;
      total += neg+zero+posi;
      tpos  += posi;
      tneg  += neg;

      for (UInt ii=0; ii<p_terms.size(); ii++) {
        std::cout << " + (";
        p_terms[ii]->write(std::cout);
        std::cout << " + " << p_const[ii] << ")^2";
      }
      for (UInt ii=0; ii<n_terms.size(); ii++) {
        std::cout << " - (";
        n_terms[ii]->write(std::cout);
        std::cout << " + " << n_const[ii] << ")^2";
      }
      std::cout << " + ";
      lin_terms->write(std::cout);
      std::cout << " + " << c << std::endl;

      if (cPtr->getUb() == INFINITY) {
        // means redundant.
      } else if (cPtr->getLb() == -INFINITY) {
        // xQx + cx <= b
        ineq++;
        if (neg==0) {
          cx++;
        } else if (posi==0 && neg>1) {
          cv++;
        } else if (neg==1) {
          cx1++;
          if (lin_terms->getNumTerms()==0) {
            cx1q++;
            if (cPtr->getUb() - c <= 0) {
              cx1qc++;
            }
          } 
        } else {
          // neg > 1 && posi>0
        }
      } else if (fabs(cPtr->getLb() - cPtr->getUb()) <= 1e-7) {
        eq++;
        if (posi==0 || neg==0) {
          eqp++;
        }
        if ((neg>0 && posi==1) || (neg==1 && posi>0)) {
          eq1++;
          if (lin_terms->getNumTerms()==0) {
            eq1q++;
          }
        }
      } else {
        // range constraint
        std::cout << cPtr->getLb() << " " << cPtr->getUb() << std::endl;
        assert (!"range constraint!");
      }
    }
  }

  delete eCalc;

 //  std::cout << "quadratic constraints = " << qc << std::endl;
 //  std::cout << "total number of eigen values = " << total << std::endl;
 //  std::cout << "total number of positive eigen values = " << tpos << std::endl;
 //  std::cout << "total number of negative eigen values = " << tneg << std::endl;
 //  std::cout << "inequalities with quadratic = " << ineq << std::endl;
 //  std::cout << "convex constraints = " << cx << std::endl;
 //  std::cout << "concave constraints = " << cv << std::endl;
 //  std::cout << "inequalities with one negative eigen value = " 
 //    << cx1 << std::endl;
 //  std::cout << "inequalities with one negative eigen value and "
 //    << "no linear part = " << cx1q << std::endl;
 //  std::cout << "inequalities with one negative eigen value, "
 //    << "no linear part and rhs non-positive= " << cx1qc << std::endl;

 //  std::cout << "equalities with quadratic = " << eq << std::endl;
 //  std::cout << "equalities with all positive eigen values = " << eqp 
 //    << std::endl;
 //  std::cout << "equalities with one negative eigen value " << eq1 << std::endl;
 //  std::cout << "equalities with one negative eigen value and "
 //    << "no linear part = " << eq1q << std::endl;

  numSOCs = cx1qc;
}

ProblemPtr createProblem(UInt mybit, ProblemPtr inst)
{
  ProblemPtr newP = (ProblemPtr) new Problem();
  VariablePtr vPtr, vCopy;
  ConstVariablePtr constVPtr;
  VariableGroupConstIterator vIter;

  ConstConstraintPtr constCPtr;
  ConstraintPtr cCopy;

  LinearFunctionPtr lPtr, lCopy;
  NonlinearFunctionPtr nlPtr, nlCopy;
  QuadraticFunctionPtr qPtr, qCopy;
  FunctionPtr fPtr;
  ObjectivePtr oPtr, oCopy;

  std::vector<VariablePtr> vars;
  std::vector<ConstraintPtr> cons;
  std::vector<ObjectivePtr> objs;
  // create the formulation from the problem. they are same in the case of
  // NLP.

  // add variables
  for (UInt i=0; i<inst->getNumVars(); i++) {
    constVPtr = inst->getVariable(i);
    vCopy = newP->newVariable(constVPtr->getLb(), constVPtr->getUb(), Continuous),
    vars.push_back(vCopy);
  }
  
  // add linear constraints
  qCopy = QuadraticFunctionPtr(); // NULL
  nlCopy = NonlinearFunctionPtr(); // NULL
  for (UInt i=0; i<inst->getNumCons(); i++) {
    constCPtr = inst->getConstraint(i);
    if (!constCPtr) {
      continue;
    }
    if (constCPtr->getQuadraticFunction() || constCPtr->getNonlinearFunction()) {
      continue;
    }
    // linear part
    lPtr = constCPtr->getLinearFunction();
    if (lPtr) {
      lCopy = (LinearFunctionPtr) new LinearFunction();
      for (vIter = lPtr->termsBegin(); vIter != lPtr->termsEnd(); vIter++) {
        constVPtr = vIter->first;
        //std::cout << "original id = " << constVPtr->getId() << std::endl;
        lCopy->addTerm(vars[constVPtr->getId()], vIter->second);
      }
    } else {
      lCopy = LinearFunctionPtr(); // NULL
    }
    fPtr = (FunctionPtr) new Function(lCopy, qCopy, nlCopy);
    newP->newConstraint(fPtr, constCPtr->getLb(), constCPtr->getUb(), 
        constCPtr->getName());
  }

  // add objective
  oPtr = inst->getObjective();
  if (oPtr->getObjectiveType() == Maximize) {
    assert(!"Minotaur not yet ready for maximization. Try minimization!");
  }

  lPtr = oPtr->getLinearFunction();
  if (lPtr) {
    lCopy = (LinearFunctionPtr) new LinearFunction();
    for (vIter = lPtr->termsBegin(); vIter != lPtr->termsEnd(); vIter++) {
      constVPtr = vIter->first;
      lCopy->addTerm(vars[constVPtr->getId()], vIter->second);
    }
  } else {
    lCopy = LinearFunctionPtr(); // NULL
  }

  // quadratic constraints
  for (UInt i=0; i<inst->getNumCons(); i++) {
    constCPtr = inst->getConstraint(i);
    if (constCPtr->getQuadraticFunction()) {
      //std::cout << "found quadratic my bit = " << mybit << std::endl;
      EigenCalculator *eCalc = new EigenCalculator();
      double c = 0;
      std::vector <LinearFunctionPtr> p_terms, n_terms;
      std::vector <double> p_const, n_const;
      LinearFunctionPtr lin_terms;
      eCalc->getSumOfSquares (p_terms, n_terms, p_const, n_const,
          lin_terms, c, constCPtr->getQuadraticFunction(), 
          constCPtr->getLinearFunction());
      assert(n_terms.size() < 2);
      //std::cout << "Number of negative values = " << n_terms.size() << std::endl;

      LSOCPtr soc;
      //std::cout << "mybit = " << mybit << "\n";
      if (mybit%2==0) {
        // create a new nonlinear constraint of type lSOC
        // sum li^2 + K - l0^2  <= rhs
        //std::cout << "doing positive = " << mybit << "\n";
        soc = (LSOCPtr) new LSOC(p_terms, n_terms[0],
            p_const, n_const[0], -constCPtr->getUb()+c);
        newP->newConstraint(soc, -INFINITY, n_const[0]);
      } else {
        // sum li^2 + K - l0^2 <= rhs
        //std::cout << "doing negative = " << mybit%2 << "\n";
        LinearFunctionPtr neg_n_term = -1*n_terms[0];
        double n_neg_const = -1*n_const[0];
        soc = (LSOCPtr) new LSOC(p_terms, neg_n_term,
            p_const, n_neg_const, -constCPtr->getUb()+c);
        newP->newConstraint(soc, -INFINITY, n_neg_const);
      }
      mybit = mybit >> 1;
    } else {
      //std::cout << "found no quadratic\n";
    }
  }

  qCopy = QuadraticFunctionPtr(); // NULL
  nlCopy = NonlinearFunctionPtr(); // NULL
  fPtr = (FunctionPtr) new Function(lCopy, qCopy, nlCopy);
  newP->newObjective(fPtr, 0, Minimize, oPtr->getName()); 

  // all done.
  //newP->write(std::cout);

  return newP;
}




