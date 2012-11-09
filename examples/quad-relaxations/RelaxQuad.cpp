//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 -- 2010 The MINOTAUR Team.
//


// /**
// \file RelaxQuad.cpp
// \brief Implement class for creating different relaxations of a quadratic
// optimization problem.
// \author Ashutosh Mahajan, Argonne National Laboratory
//
// Implement class for creating different relaxations of a quadratic
// optimization problem.
//
// */

#include "MinotaurConfig.h"
#include "Operations.h"
#include "RelaxQuad.h"
#include "LPRelaxation.h"
#include "NLPRelaxation.h"
#include "Types.h"

#include "MinotaurDeconfig.h"
#include "OsiLPEngine.h"
#undef F77_FUNC_
#include "MinotaurConfig.h"

#define QINF 1e20

using namespace Minotaur;

RelaxQuad::RelaxQuad(ProblemPtr problem, OptionDBPtr options)
  : problem_(problem),
    rel_(RelaxationPtr()), // NULL
    options_(options),
    etol_(1e-6)
{
}


RelaxQuad::~RelaxQuad()
{
  problem_.reset();
  options_.reset();
  rel_.reset();
}


void RelaxQuad::createRelaxation(RelaxType r_type)
{
  if (rel_) {
    rel_.reset();
  }
  switch (r_type) {
   case (TERMBYTERM):
     termByTerm_();// each term individually, convex terms as nonlinear.
     break;
   case (TERMBYTERM_OA):
     termByTermOA_(); // linear
     break;
   case (TERMBYTERM_OA_CUTS):
     termByTermOA_(); // same setup as above.
     break;
   default:
     break;
  }
  // EV_OA_SECANT,      // Eigen Value, convex terms linearized.
  // EV_SECANT,         // Eigen Value, convex terms nonlinear.
  // EV_OA_CUTS         // Eigen Value, convex terms linearized, 
                        // intersection cuts.
}


void RelaxQuad::termByTermOA_()
{
  // visit each constraint and identify constraints of the form 
  // x^2 - w <=0
  // replace them with OA at (0,0)
  ConstVariablePtr v1, v2;
  ConstraintPtr cPtr;
  LinearFunctionPtr lPtr;
  QuadraticFunctionPtr qPtr;

  sq_vars_.clear();
  neg_sq_vars_.clear();
  termByTerm_(false);
  //for (UInt i=0; i<sq_vars_.size(); ++i) {
  //  v1 = sq_vars_[i].first;
  //  v2 = sq_vars_[i].second;
  //  v1->writeName(std::cout);
  //  std::cout << " ";
  //  v2->writeName(std::cout);
  //  std::cout << "\n";
  //}
}


void RelaxQuad::termByTerm_(Bool keep_squares)
{
  VariablePtr vPtr, vCopy, v3, v4;
  ConstVariablePtr constVPtr;
  ConstraintPtr cPtr;
  VariableConstIterator v1, v2;
  VariableGroupConstIterator vg;
  LinearFunctionPtr lPtr, lCopy, lCopy2;
  QuadraticFunctionPtr qPtr, qCopy, qCopy2;
  NonlinearFunctionPtr nlPtr = NonlinearFunctionPtr();  // NULL
  Double lb, ub, vlb, vub, rhs;
  FunctionPtr f;
  ObjectivePtr oPtr;
  Bool is_bound;

  if (keep_squares) {
    rel_ = (NLPRelaxationPtr) new NLPRelaxation();
  } else {
    rel_ = (LPRelaxationPtr) new LPRelaxation();
  }

  // copy all variables.
  v1 = problem_->varsBegin();
  v2 = problem_->varsEnd();
  rel_->newVariables(v1, v2);

  // copy all linear constraints.
  for (ConstraintConstIterator cit=problem_->consBegin();
      cit!=problem_->consEnd(); ++cit) {
    cPtr = *cit;
    lb = cPtr->getLb();
    ub = cPtr->getUb();

    // linear part
    lPtr = cPtr->getLinearFunction();
    lCopy = (LinearFunctionPtr) new LinearFunction();
    if (lPtr) {
      for (vg = lPtr->termsBegin(); vg != lPtr->termsEnd(); ++vg) {
        constVPtr = vg->first;
        lCopy->addTerm(rel_->getVariable(constVPtr->getId()), vg->second);
      }
    } else {
       // nothing
    }

    // quadratic part
    qPtr = cPtr->getQuadraticFunction();

    if (qPtr) {
      if (lb > -QINF) {
        // range constraint. We get 1 additional variable and 2 additional 
        // constraints per quadratic term.
        for (VariablePairGroupConstIterator it = qPtr->begin(); 
            it != qPtr->end(); ++it) {
          v3 = rel_->getVariable(it->first.first->getId());
          v4 = rel_->getVariable(it->first.second->getId());
          if (v3==v4) {
            // we have a square term. Doesn't matter if it is positive 
            // or negative. add a new variable
            BoundsOnSquare(v3, vlb, vub);
            vPtr = rel_->newVariable(vlb, vub, Continuous);

            // v3^2 - vPtr <= 0.
            sq_vars_.push_back(std::make_pair(v3, vPtr));
            if (keep_squares) {
              lCopy2 = (LinearFunctionPtr) new LinearFunction();
              lCopy2->addTerm(vPtr, -1.);
              qCopy  = (QuadraticFunctionPtr) new QuadraticFunction(); 
              qCopy->addTerm(std::make_pair(v3, v3), 1.);
              f = (FunctionPtr) new Function(lCopy2, qCopy, nlPtr);
              rel_->newConstraint(f, -INFINITY, 0.);
            }

            // secant constraint
            neg_sq_vars_.push_back(std::make_pair(v3, vPtr));
            f = getSecantConstraint_(v3, vPtr, rhs, is_bound);
            if (is_bound==false) {
              rel_->newConstraint(f, -INFINITY, rhs);
            }

            // add vPtr to the linear terms of existing constraint
            lCopy->addTerm(vPtr, it->second);

          } else {
            // v3 != v4. McCormick is here!
            // v3.v4 = w
            BoundsOnProduct(v3, v4, vlb, vub);
            vPtr = rel_->newVariable(vlb, vub, Continuous);

            // add the new variable to the current constraint.
            lCopy->addTerm(vPtr, it->second);

            // w >= l4v3 + l3v4 - l3l4
            lCopy2 = (LinearFunctionPtr) new LinearFunction();
            lCopy2->addTerm(v3, v4->getLb());
            lCopy2->addTerm(v4, v3->getLb());
            lCopy2->addTerm(vPtr, -1.);
            rhs = v3->getLb() * v4->getLb();
            f = (FunctionPtr) new Function(lCopy2);
            rel_->newConstraint(f, -INFINITY, rhs);

            // w >= u4v3 + u3v4 - u3u4
            lCopy2 = (LinearFunctionPtr) new LinearFunction();
            lCopy2->addTerm(v3, v4->getUb());
            lCopy2->addTerm(v4, v3->getUb());
            lCopy2->addTerm(vPtr, -1.);
            rhs = v3->getUb() * v4->getUb();
            f = (FunctionPtr) new Function(lCopy2);
            rel_->newConstraint(f, -INFINITY, rhs);

            // w <= u4v3 + l3v4 - l3u4
            lCopy2 = (LinearFunctionPtr) new LinearFunction();
            lCopy2->addTerm(v3, -(v4->getUb()));
            lCopy2->addTerm(v4, -(v3->getLb()));
            lCopy2->addTerm(vPtr, 1.);
            rhs = -1. * v4->getUb() * v3->getLb();
            f = (FunctionPtr) new Function(lCopy2);
            rel_->newConstraint(f, -INFINITY, rhs);

            // w <= l4v3 + u3v4 - u3l4
            lCopy2 = (LinearFunctionPtr) new LinearFunction();
            lCopy2->addTerm(v3, -(v4->getLb()));
            lCopy2->addTerm(v4, -(v3->getUb()));
            lCopy2->addTerm(vPtr, 1.);
            rhs = -1. * v4->getLb() * v3->getUb();
            f = (FunctionPtr) new Function(lCopy2);
            rel_->newConstraint(f, -INFINITY, rhs);
          }
        }
      } else {
        // Inequality constraint (no range). 
        for (VariablePairGroupConstIterator it = qPtr->begin(); 
            it != qPtr->end(); ++it) {
          v3 = rel_->getVariable(it->first.first->getId());
          v4 = rel_->getVariable(it->first.second->getId());
          if (v3==v4) {
            // we have a square term. It matters if it is positive 
            // or negative. add a new variable nevertheless
            BoundsOnSquare(v3, vlb, vub);
            vPtr = rel_->newVariable(vlb, vub, Continuous);

            if (it->second>0) {
              // +ive square. Only need OA or convex constraint. Add
              // v3^2 -vPtr <= 0
              sq_vars_.push_back(std::make_pair(v3, vPtr));
              if (keep_squares) {
                lCopy2 = (LinearFunctionPtr) new LinearFunction();
                lCopy2->addTerm(vPtr, -1.);
                qCopy  = (QuadraticFunctionPtr) new QuadraticFunction(); 
                qCopy->addTerm(std::make_pair(v3, v3), 1.);
                f = (FunctionPtr) new Function(lCopy2, qCopy, nlPtr);
                rel_->newConstraint(f, -INFINITY, 0.);
              }
              
              // add vPtr to the linear terms of existing constraint
              lCopy->addTerm(vPtr, it->second);
              
            } else if (it->second<0) {
              // add secant constraint
              neg_sq_vars_.push_back(std::make_pair(v3, vPtr));
              f = getSecantConstraint_(v3, vPtr, rhs, is_bound);
              if (is_bound==false) {
                rel_->newConstraint(f, -INFINITY, rhs);
              }

              // add vPtr to the linear terms of existing constraint
              lCopy->addTerm(vPtr, it->second);
            }
          } else {
            // v3 != v4. McCormick is here!
            // v3.v4 = w
            BoundsOnProduct(v3, v4, vlb, vub);
            vPtr = rel_->newVariable(vlb, vub, Continuous);

            // add the new variable to the current constraint.
            lCopy->addTerm(vPtr, it->second);

            if (it->second>0) {
              // lf + w <= rhs. We only need w>=... inequalities.
              // w >= l4v3 + l3v4 - l3l4
              lCopy2 = (LinearFunctionPtr) new LinearFunction();
              lCopy2->addTerm(v3, v4->getLb());
              lCopy2->addTerm(v4, v3->getLb());
              lCopy2->addTerm(vPtr, -1.);
              rhs = v3->getLb() * v4->getLb();
              f = (FunctionPtr) new Function(lCopy2);
              rel_->newConstraint(f, -INFINITY, rhs);

              // w >= u4v3 + u3v4 - u3u4
              lCopy2 = (LinearFunctionPtr) new LinearFunction();
              lCopy2->addTerm(v3, v4->getUb());
              lCopy2->addTerm(v4, v3->getUb());
              lCopy2->addTerm(vPtr, -1.);
              rhs = v3->getUb() * v4->getUb();
              f = (FunctionPtr) new Function(lCopy2);
              rel_->newConstraint(f, -INFINITY, rhs);
            } else if (it->second<0) {
              // lf + w <= rhs. We only need w<=... inequalities.
              // w <= u4v3 + l3v4 - l3u4
              lCopy2 = (LinearFunctionPtr) new LinearFunction();
              lCopy2->addTerm(v3, -(v4->getUb()));
              lCopy2->addTerm(v4, -(v3->getLb()));
              lCopy2->addTerm(vPtr, 1.);
              rhs = -1. * v4->getUb() * v3->getLb();
              f = (FunctionPtr) new Function(lCopy2);
              rel_->newConstraint(f, -INFINITY, rhs);

              // w <= l4v3 + u3v4 - u3l4
              lCopy2 = (LinearFunctionPtr) new LinearFunction();
              lCopy2->addTerm(v3, -(v4->getLb()));
              lCopy2->addTerm(v4, -(v3->getUb()));
              lCopy2->addTerm(vPtr, 1.);
              rhs = -1. * v4->getLb() * v3->getUb();
              f = (FunctionPtr) new Function(lCopy2);
              rel_->newConstraint(f, -INFINITY, rhs);
            }
          }
        } // for loop
      }
    } else { // if (qPtr)
      // nothing
    }

    // add the linear constraint
    qCopy = QuadraticFunctionPtr(); // NULL
    f = (FunctionPtr) new Function(lCopy, qCopy, nlPtr);
    rel_->newConstraint(f, lb, ub);

    // nonlinear part
    if (cPtr->getNonlinearFunction()) {
      assert(!"non linear functions not allowed!");
    }
  }

  // copy Objective
  oPtr = problem_->getObjective();
  if (oPtr->getObjectiveType() == Maximize) {
    assert(!"Minotaur not yet ready for maximization. Try minimization!");
  }

  lPtr = oPtr->getLinearFunction();
  if (lPtr) {
    lCopy = (LinearFunctionPtr) new LinearFunction();
    for (vg = lPtr->termsBegin(); vg != lPtr->termsEnd(); vg++) {
      constVPtr = vg->first;
      lCopy->addTerm(rel_->getVariable(constVPtr->getId()), vg->second);
    }
  } else {
    lCopy = LinearFunctionPtr(); // NULL
  }

  qCopy = QuadraticFunctionPtr(); // NULL
  f = (FunctionPtr) new Function(lCopy, qCopy, nlPtr);
  rel_->newObjective(f, 0, Minimize, oPtr->getName());

  //rel_->write(std::cout);
  std::cout << std::endl;
}


void RelaxQuad::solveUsingOA(LPEnginePtr lp_e)
{
  const Double *e_sol;
  UInt i1, i2;
  VariablePtr v1, v2;
  Double xval, wval;
  FunctionPtr f;
  Double rhs;
  ConstraintPtr cPtr;
  UInt new_cons;
  UInt total_new_cons=0;

  //rel_->write(std::cout);
  lp_e->load(rel_);
  lp_e->solve();
  std::cout << "Solution value before Mc+Secant+OA = " 
    << lp_e->getSolutionValue() << std::endl;

  while(true) {
    new_cons = 0;
    e_sol = lp_e->getSolution();
    // check for violated constraints of type: x^2 - w <= 0
    for (UInt i=0; i<sq_vars_.size(); ++i) {
      v1 = rel_->getVariable(sq_vars_[i].first->getId());
      v2 = rel_->getVariable(sq_vars_[i].second->getId());
      i1 = v1->getId();
      i2 = v2->getId();
      xval = e_sol[i1];
      wval = e_sol[i2];
      if (xval*xval - wval > etol_) {
        //std::cout << "violation: " << v1->getName() << " = " << xval <<
        //  " " << v2->getName() << " = " << wval << std::endl;
        f = getOAConstraint_(v1, v2, xval, wval, rhs);
        cPtr =  rel_->newConstraint(f, -INFINITY, rhs);
        //cPtr->write(std::cout);
        ++new_cons;
        ++total_new_cons;
      }
    }
    lp_e->load(rel_);
    lp_e->solve();
    //std::cout << lp_e->getStatusString() << std::endl;
    //std::cout << "Solution value of Mc+Secant+OA = " 
    //  << lp_e->getSolutionValue() << std::endl;
    if (new_cons==0 || lp_e->getStatus() != ProvenOptimal) {
      break;
    }
  }
  std::cout << "total new cons = " << total_new_cons << std::endl;
  std::cout << "Solution value after Mc+Secant+OA = " 
    << lp_e->getSolutionValue() << std::endl;
  std::cout << "Engine status after Mc+Secant+OA = " 
    << lp_e->getStatusString() << std::endl;
}


void RelaxQuad::solveUsingOA_Cuts(LPEnginePtr lp_e)
{
  const Double *e_sol;
  UInt i1, i2;
  VariablePtr v1, v2;
  Double xval, wval;
  FunctionPtr f;
  Double rhs;
  ConstraintPtr cPtr;
  UInt new_cons;
  UInt total_new_cons=0;

  lp_e->load(rel_);
  lp_e->solve();
  std::cout << "Solution value before Mc+Secant+OA+Cuts = " 
    << lp_e->getSolutionValue() << std::endl;
  while(true) {
    new_cons = 0;
    e_sol = lp_e->getSolution();
    for (UInt i=0; i<neg_sq_vars_.size(); ++i) {
      v1 = rel_->getVariable(neg_sq_vars_[i].first->getId());
      v2 = rel_->getVariable(neg_sq_vars_[i].second->getId());
      i1 = v1->getId();
      i2 = v2->getId();
      xval = e_sol[i1];
      wval = e_sol[i2];
      //std::cout << xval << " " << wval << std::endl;
      //v1->writeName(std::cout);
      //std::cout << " ";
      //v2->writeName(std::cout);
      if (wval-xval*xval > etol_) {
        std::cout << "-ive square violation: " << v1->getName() << " = " << xval <<
          " " << v2->getName() << " = " << wval << std::endl;
        f = getICut_(v1, v2, xval, wval, rhs, lp_e);
        cPtr =  rel_->newConstraint(f, -INFINITY, rhs);
        //cPtr->write(std::cout);
        ++new_cons;
        ++total_new_cons;
        lp_e->load(rel_);
        lp_e->solve();
        std::cout << "Solution value after Mc+Secant+OA+Cuts = " 
          << lp_e->getSolutionValue() << std::endl;
        std::cout << "Engine status after Mc+Secant+OA+Cuts = " 
          << lp_e->getStatusString() << std::endl;
      }
    }
    if (new_cons==0 || lp_e->getStatus() != ProvenOptimal) {
      break;
    }
  }
  std::cout << "total new intersection cons = " << total_new_cons << std::endl;
  std::cout << "Solution value after Mc+Secant+OA+Cuts = " 
    << lp_e->getSolutionValue() << std::endl;
  std::cout << "Engine status after Mc+Secant+OA+Cuts = " 
    << lp_e->getStatusString() << std::endl;
}


FunctionPtr RelaxQuad::getSecantConstraint_(VariablePtr x, 
    VariablePtr y, Double &rhs, Bool &is_bound)
{
  // write secant approximation of y - x^2 <= 0 
  LinearFunctionPtr lf;
  Double lb = x->getLb();
  Double ub = x->getUb();
  is_bound = false;
  rhs = -ub*lb;
  if (fabs(ub+lb) > etol_) {
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.);
    lf->addTerm(x, -1.*(ub+lb));
  } else {
    is_bound = true;
  }

  FunctionPtr f = (FunctionPtr) new Function(lf);

  return f;
}


FunctionPtr RelaxQuad::getOAConstraint_(VariablePtr v1, 
    VariablePtr v2, Double xval, Double wval, Double &rhs)
{
  FunctionPtr f;
  LinearFunctionPtr lf;

  // x^2 - w <= 0
  // \grad g(x_k) . (x-x_k) + g(x_k) \leq 0
  // [2x_k, -1] . [x-x_k, w-w_k] + (x_k^2 - w_k) \leq 0
  // 2.x.x_k - w \leq x_k.x_k
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(v1, xval*2.);
  lf->addTerm(v2, -1.);
  rhs = xval*xval;
  f = (FunctionPtr) new Function(lf);
  
  // wval unused.
  return f;
}


FunctionPtr RelaxQuad::getICut_(VariablePtr v1, VariablePtr v2, Double xval, 
    Double wval, Double &rhs, LPEnginePtr lp_e)
{
  FunctionPtr f;
  LPEngine *lp = &(*lp_e);
  OsiLPEngine *osi_e = dynamic_cast<OsiLPEngine *>(lp);

  // 0: free
  // 1: basic
  // 2: upper
  // 3: lower
  int *cstat, *rstat;
  double *z;
  double *slack;
  LoggerPtr logger = (LoggerPtr) new Logger(LogDebug);
  int *basics;
  int nbvar;
  int *nbvars;
  bool *is_b;
  double *nbcol;
  int cnt;
  int v1_pos, v2_pos;
  double coeff1, coeff2;
  double *delta;
  LinearFunctionPtr lf, lf2, lf3;

  int n = rel_->getNumVars();
  int m = rel_->getNumCons();

  //std::cout << "writing size\n";
  rel_->setLogger(logger);
  rel_->calculateSize();
  //rel_->writeSize();
  //rel_->write(std::cout);
  cstat = new int[n];
  rstat = new int[m];
  z     = new double[n];
  slack = new double[m];
  basics = new int[m];
  nbvars = new int[n];
  is_b = new bool[m+n];
  nbcol = new double[n];
  delta = new double[m];
  int root_status = -1;

  OsiSolverInterface *osi = osi_e->getSolver();
  assert(osi);
  assert(osi->canDoSimplexInterface() == 2);

  osi->enableFactorization();

  assert (osi->basisIsAvailable());
  osi->getBasisStatus(cstat,rstat); 
  osi->getBasics(basics);

  //for (int i=0; i<n; ++i) {
  //  std::cout << " cstat = " << cstat[i];
  //}
  //for (int i=0; i<m; ++i) {
  //  std::cout << " rstat = " << rstat[i];
  //}
  //std::cout << "\n";

  //for (int r=0; r<m; ++r) {
  //  osi->getBInvARow(r,z,slack);
  //  for (int i=0; i<n; ++i) {
  //    std::cout << " z = " << z[i];
  //  }
  //  for (int i=0; i<m; ++i) {
  //    std::cout << " slack = " << slack[i] <<  " ";
  //  }
  //  std::cout << "\n";
  //}

  // x can not be at its bound. Hence it can not be in basis. It can also be
  // free.
  assert(cstat[v1->getId()] == 1 || cstat[v1->getId()] == 0);

  std::fill(is_b, is_b+n+m, false);
  v1_pos = -1;
  v2_pos = -1;
  for (int i=0; i<m; ++i) {
    is_b[basics[i]] = true;
    if (basics[i] < n) {
      //std::cout << "basic variable " << i << " = " << 
      //  rel_->getVariable(basics[i])->getName() << std::endl;
    } else if (basics[i] < m+n) {
      //std::cout << "basic variable, " << i << " = slack of row " << 
      //  basics[i]-n << std::endl;
    } else {
      assert(!"unknown basic variable!");
    }
    if (basics[i]==v1->getId()) {
      v1_pos = i;
    } else if (basics[i]==v2->getId()) {
      v2_pos = i;
    }
  }

  assert(v1_pos>-1);
  assert(v2_pos>-1);

  cnt = 0;
  for (int i=0; i<n+m; ++i) {
    if (is_b[i] == false) {
      nbvars[cnt] = i;
      ++cnt;
    }
  }
  assert(cnt == n);


  if (cstat[v2->getId()] < 2) {
    // if v2 is basic
    // for each nonbasic variable do the following
    for (int i=0; i<n; ++i) {
      // get the BInvACol of the tableau corresponding to this non-basic column.
      nbvar = nbvars[i];
      osi->getBInvACol(nbvar, nbcol);
      //std::cout << "****" << " nonbasic column " << nbvar << " ****\n";

      if (nbvar<n) {
        coeff1 = -nbcol[v1_pos];
        coeff2 = -nbcol[v2_pos];

        if (cstat[nbvar]==2) {
          //nonbasic at upper
          //std::cout << "variable " << i << " nb at upper" << std::endl;
          delta[i] = findQuadRoot_(coeff1*coeff1, 
              2*xval*coeff1-coeff2, xval*xval - wval, root_status, false);
        } else if (cstat[nbvar]==3) {
          // nonbasic at lower
          //std::cout << "variable " << i << " nb at lower" << std::endl;
          delta[i] = findQuadRoot_(coeff1*coeff1, 
              2*xval*coeff1-coeff2, xval*xval - wval, root_status, true);
        } else {
          assert(!"variable is free!");
        }

        assert(root_status>-1);
        if (root_status==0) {
          // no roots found
          delta[i]=INFINITY;
        }
        //std::cout << "coeff1 = " << coeff1 << " coeff2 = " << coeff2 << 
        //  " delta = " << delta[i] << std::endl;
      } else if (nbvar<n+m) {
        coeff1 = -nbcol[v1_pos];
        coeff2 = -nbcol[v2_pos];
        if (rstat[nbvar-n]==2) {
          //std::cout << "slack " << i << " nb at upper" << std::endl;
          delta[i] = findQuadRoot_(coeff1*coeff1, 
              2*xval*coeff1-coeff2, xval*xval - wval, root_status, false);
        } else if (rstat[nbvar-n]==3) {
          //std::cout << "slack " << i << " nb at lower" << std::endl;
          delta[i] = findQuadRoot_(coeff1*coeff1, 
              2*xval*coeff1-coeff2, xval*xval - wval, root_status, true);
        } else {
          assert(!"slack is free!");
        }
        assert(root_status>-1);
        if (root_status==0) {
          // no roots found
          delta[i]=INFINITY;
        }
        //std::cout << "coeff1 = " << coeff1 << " coeff2 = " << coeff2 << 
        //  " delta = " << delta[i] << std::endl;
      }
      //std::cout << "displaying column\n";
      //for (int j=0; j<m; ++j) {
      //  std::cout << nbcol[j] << std::endl;
      //}
    }
  } else {
    assert(!"can't do when v2 is nonbasic!");
  }

  // take all the deltas and create the cut.
  // Cut is:
  // \sum_{i|\delta_i>0} (x_i - l_i)/\delta_i  - \sum_{i|delta_i<0} (u_i -
  // x_i)/\delta_i
  // If x_i is actually a slack, then replace it with full constraint
  lf = (LinearFunctionPtr) new LinearFunction();
  rhs = 0;
  for (int i=0; i<n; ++i) {
    if (delta[i] == INFINITY) {
      continue;
    }

    nbvar = nbvars[i];

    if (nbvar<n) {
      // it is a real varable. add it.
      if (delta[i]>0) {
        lf->addTerm(rel_->getVariable(nbvar), 1./delta[i]);
        rhs += rel_->getVariable(nbvar)->getLb()*1./delta[i];
      } else if (delta[i]<0) {
        lf->addTerm(rel_->getVariable(nbvar), -1./delta[i]);
        rhs += -1.*(rel_->getVariable(nbvar)->getUb())*1./delta[i];
      }
    } else {
      // The variable is a slack. More work. Convert slack into vars and
      // constant.
      // slack = rhs-\sum_{i}a_ix_i. Slacks may or may not have both bounds.
      lf2 = rel_->getConstraint(nbvar-n)->getLinearFunction();
      assert(lf2); // why else this slack?
      lf3 = -1./fabs(delta[i]) * lf2;
      *lf += lf3;
      if (delta[i]>0) {
        rhs -= rel_->getConstraint(nbvar-n)->getUb()/delta[i];
      } else if (delta[i]<0) {
        rhs += rel_->getConstraint(nbvar-n)->getUb()/delta[i];
        rhs -= (rel_->getConstraint(nbvar-n)->getUb() - 
            rel_->getConstraint(nbvar-n)->getLb())/delta[i];
      }
    }
    //std::cout << "rhs = " << rhs << " nbvar = " << nbvar << std::endl;
  }
  rhs += 1.;
  (*lf) *= -1.;
  rhs *= -1.;
  osi->disableFactorization();

  delete [] cstat;
  delete [] rstat;
  delete [] z;
  delete [] slack;
  delete [] nbcol;
  delete [] nbvars;
  delete [] is_b;
  f = (FunctionPtr) new Function(lf);
  f->write(std::cout);
  std::cout << " <= " << rhs << std::endl;
  return f;
}


// root_status:
//  1: root of correct sign found
//  0: roots found but not of right sign.
// -1: error
Double RelaxQuad::findQuadRoot_(Double a, Double b, Double c, Int &root_status,
   Bool positive_root)
{
  Double sq_disc;

  root_status = 1;
  // a is always positive
  assert( a > -etol_);

  // c is always negative.
  assert( c < etol_);
  if (c > 0) {
    c=0;
  }

  //std::cout << "a = " << a << " b = "  << b << " c = " << c << std::endl;
  if (fabs(a)<etol_) {
    // a is zero, it is a linear equation,
    if (fabs(c)<etol_) {
      return 0;
    } else if (fabs(b)<etol_) {
      root_status = 0;
      return 0;
    }
    if ((positive_root && b<0) || (!positive_root & b>0)) {
      root_status = 0;
      return 0;
    }
    return -c/b;
  }

  // b^2 - 4ac must be positive. Otherwise no roots. We have already
  // implicitly checked this condition above.
  sq_disc = b*b - 4*a*c;
  //std::cout << "desc = "<<  sq_disc << std::endl;

  if (sq_disc<0) {
    sq_disc=0;
  }

  if (positive_root==true) {
    return (-b + sqrt(sq_disc))/2./a;
  } else {
    return (-b - sqrt(sq_disc))/2./a;
  }
  root_status = -1;
  return 0;
}

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
