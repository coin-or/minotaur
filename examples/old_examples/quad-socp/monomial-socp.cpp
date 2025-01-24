//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
// 

/** 
 This is example driver code to convert monomial constraints into socps if
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

#include "Polynomial.h"
#include "opcode.hd"

using namespace Minotaur;

ProblemPtr build_instance(char *fname, VariableSet & pvars_, PolyVector & pVec);
void addLinearConstraint_(int i, ProblemPtr instance, ASL *myAsl_, std::vector<VariablePtr> & vars_);
void addLinearObjective_(int i, ProblemPtr instance, ASL *myAsl_, std::vector<VariablePtr> & vars_);
void addLinearTermsFromConstr_(LinearFunctionPtr & lf, int i, ASL *myAsl_, std::vector<VariablePtr> & vars_);
void addPolynomialConstraint_(int i, ProblemPtr instance,
    ASL *myAsl_, std::vector<VariablePtr> & vars_, VariableSet & pvars_, 
    PolyVector & pVec);
void getPolynomial_(PolynomialPtr & pPtr, double & c, expr *e_ptr, ASL *myAsl_, std::vector<VariablePtr> & vars_);
bool should_solve(UInt mybit, PolyVector & pVec);
double solve_nlp(UInt mybit, ProblemPtr inst, PolyVector & pVec);
ProblemPtr createProblem(UInt mybit, ProblemPtr inst, PolyVector & pVec);
void addLinearTermsFromObj_(LinearFunctionPtr & lf, int i, ASL *myAsl_, std::vector<VariablePtr> & vars_);
void make_pairs(UInt &mybit2, ConstPolynomialPtr pPtr, ProblemPtr newP, std::vector<ConstVariablePair> & pPairs, std::vector<ConstVariablePair> & nPairs);
void add_socs(std::vector<ConstVariablePair> & pPairs, std::vector<ConstVariablePair> & nPairs, ProblemPtr newP, double rhs);

void usage()
{
  std::cout << "usage: ./monomial-socp {name-of-.nl-file}\n";
}


int main(int argc, char** argv)
{
  UInt numSOCs;
  UInt nlps_solved = 0;
  UInt inf_nlps = 0;
  TimerFactory *tFactory = new TimerFactory();     
  Timer *timer=tFactory->GetTimer();
  VariableSet pvars_;
  PolyVector pVec;
  VariablePtr vPtr;
  std::vector<ConstVariablePtr> aux_vars;

  timer->Start();
  double best_value = INFINITY;

  if (argc < 2) {
    usage();
    return -1;
  }
  ProblemPtr inst, newInst;
  inst = build_instance(argv[1], pvars_, pVec);
  //for (VarSetConstIterator iter=pvars_.begin(); iter!=pvars_.end(); iter++) {
  //  (*iter)->writeName(std::cout);
  //  std::cout << std::endl;
  //}
  std::cout << "no of vars = " << pvars_.size() << std::endl;

  for (PolyVectorIterator iter=pVec.begin(); iter!=pVec.end(); iter++) {
    (*iter)->write(std::cout);
    std::cout << std::endl;
    // factorize them.
    (*iter)->factorize();
  }

  // add a new variable, one for each factor in a polynomial
  UInt newvars = 0;
  for (PolyVectorIterator iter=pVec.begin(); iter!=pVec.end(); iter++) {
    std::vector<double>::const_iterator citer = (*iter)->lfsCBegin();
    bool check_neg = true;
    for (std::vector<ConstLinearFunctionPtr>::const_iterator liter=(*iter)->lfsBegin(); liter!=(*iter)->lfsEnd();
        liter++, citer++) {
      // add new variables
      char buf[64];
      snprintf(buf, 64, "aux_v_%d", newvars);
      std::string vName = std::string(buf);
      vPtr = inst->newVariable(-INFINITY, INFINITY, Continuous, vName);
      aux_vars.push_back(vPtr);
      (*iter)->addAuxVar(vPtr);
      newvars++;

      // add linear constraint: lf_0 - aux_0 = -const_0
      snprintf(buf, 64, "aux_c_%d", newvars);
      std::string cName = std::string(buf);
      LinearFunctionPtr lf = (*liter)->clone();
      if (check_neg) {
        check_neg = false;
        if ((*iter)->isNeg()) {
          (*lf) *= -1.0;
        }
        lf->addTerm(vPtr, -1.0);
        FunctionPtr fptr = (FunctionPtr) new Function(lf);
        inst->newConstraint(fptr, (*citer), (*citer), cName);
      } else {
        lf->addTerm(vPtr, -1.0);
        FunctionPtr fptr = (FunctionPtr) new Function(lf);
        inst->newConstraint(fptr, -(*citer), -(*citer), cName);
      }
    }
  }
  std::cout << "new variables = " << newvars << std::endl;
  UInt finalbit = 1 << newvars;
  std::cout << "max no of nodes required = " << finalbit << std::endl;
  //inst->write(std::cout);

  // start branch and bound
  UInt mybit = 0;
  UInt ignored = 0;
  double obj, best_obj=INFINITY;
  while (mybit < finalbit) {
    if (should_solve(mybit, pVec)) {
      obj = solve_nlp(mybit, inst, pVec);
      if (obj < best_obj) {
        best_obj = obj;
        std::cout << "best_obj = " << best_obj << std::endl;
      }
      nlps_solved++;
    } else {
      ignored++;
    }
    mybit++;
  }
  std::cout << "nlps solved = " << nlps_solved << std::endl;
  std::cout << "nlps ignored = " << ignored << std::endl;
  std::cout << "best solution value = " << best_obj << std::endl;
  std::cout << "time used = " << timer->Query() << std::endl;

  delete timer;
  delete tFactory;
  return 0;
}

ProblemPtr build_instance(char *fname2, VariableSet & pvars_, PolyVector & pVec)
{
  ProblemPtr instance = ProblemPtr();
  FILE *nlff = NULL;
  ASL *myAsl_;
  std::string fsname(fname2);
  myAsl_ = ASL_alloc(ASL_read_fg); 
  nlff = jac0dim_ASL(myAsl_, fname2, fsname.length());

  UInt nVars = myAsl_->i.n_var_;
  UInt nCons = myAsl_->i.n_con_;
  myAsl_->i.X0_ = (real *)mymalloc_ASL(nVars*sizeof(real));
  fg_read_ASL(myAsl_,nlff,0);

  instance= (ProblemPtr) new Problem();

  int stop_index, start_index;
  VariablePtr vPtr;
  std::string vName;
  std::vector <VariablePtr> vars_;

  // first visit all nonlinear variables
  // continuous nonlinear variables in both obj and cons
  start_index = 0;
  stop_index = myAsl_->i.nlvb_ - myAsl_->i.nlvbi_;
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Continuous, vName);
    vars_.push_back(vPtr);
  }
  // integer nonlinear variables in both obj and cons
  start_index = stop_index;
  stop_index += myAsl_->i.nlvbi_;
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Integer, vName);
    vars_.push_back(vPtr);
  }
  // continuous nonlinear variables in cons only
  start_index = stop_index;
  stop_index += myAsl_->i.nlvc_ - (myAsl_->i.nlvb_ + myAsl_->i.nlvci_);
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Continuous, vName);
    vars_.push_back(vPtr);
  }
  // integer nonlinear variables in cons only
  start_index = stop_index;
  stop_index += myAsl_->i.nlvci_;
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Integer, vName);
    vars_.push_back(vPtr);
  }

  if (myAsl_->i.nlvo_ > myAsl_->i.nlvc_) {
    // there are some variables that are linear in constraints but nonlinear
    // in objective. the first 'nlvc' are already counted.
    // continuous nonlinear variables in obj only
    start_index = stop_index;
    stop_index += myAsl_->i.nlvo_ - (myAsl_->i.nlvc_ + myAsl_->i.nlvoi_);
    for (int i=start_index; i<stop_index; i++) {
      vName = std::string(var_name_ASL(myAsl_, i));
      vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
          myAsl_->i.LUv_[2*i+1], Continuous, vName);
      vars_.push_back(vPtr);
    }
    // integer nonlinear variables in obj only
    start_index = stop_index;
    stop_index += myAsl_->i.nlvoi_;
    for (int i=start_index; i<stop_index; i++) {
      vName = std::string(var_name_ASL(myAsl_, i));
      vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
          myAsl_->i.LUv_[2*i+1], Integer, vName);
      vars_.push_back(vPtr);
    }

    assert ((int) stop_index == myAsl_->i.nlvo_);
  } else {
    // all variables that are nonlinear in objective are also nonlinear in
    // constraints and hence we dont need to add anything more.
    assert ((int) stop_index == myAsl_->i.nlvc_);
  } // all nonlinear variables have been added.


  // we dont deal with linear arcs for now.
  assert(myAsl_->i.nwv_ == 0);

  // visit all linear continuous variables
  // continuous linear variables
  start_index = stop_index;
  stop_index  = myAsl_->i.n_var_ - (myAsl_->i.niv_ + myAsl_->i.nbv_);
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Continuous, vName);
    vars_.push_back(vPtr);
  }
  // integer linear variables
  start_index = stop_index;
  stop_index += myAsl_->i.niv_;
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Integer, vName);
    vars_.push_back(vPtr);
  }
  // binary linear variables
  start_index = stop_index;
  stop_index += myAsl_->i.nbv_;
  for (int i=start_index; i<stop_index; i++) {
    vName = std::string(var_name_ASL(myAsl_, i));
    vPtr = instance->newVariable(myAsl_->i.LUv_[2*i], 
        myAsl_->i.LUv_[2*i+1], Binary, vName);
    vars_.push_back(vPtr);
  }

  assert ((int) stop_index == myAsl_->i.n_var_);

  // add linear constraints 
  start_index = myAsl_->i.nlc_ - myAsl_->i.nlnc_;
  stop_index  = myAsl_->i.n_con_;
  for (int i=start_index; i<stop_index; i++) {
    addLinearConstraint_(i, instance, myAsl_, vars_);
  }

  // visit each constraint and copy the quadratic and linear parts.
  // add quadratic constraints 
  start_index = 0;
  stop_index = myAsl_->i.nlc_ - myAsl_->i.nlnc_;
  for (int i=start_index; i<stop_index; i++) {
    addPolynomialConstraint_(i, instance, myAsl_, vars_, pvars_, pVec);
  }

  if (myAsl_->i.n_obj_ > 0) {
      addLinearObjective_(0, instance, myAsl_, vars_);
  }
  return instance;
}

void addLinearConstraint_(int i, ProblemPtr instance, ASL *myAsl_, std::vector<VariablePtr> & vars_)
{
  std::string cName;
  LinearFunctionPtr lfPtr = LinearFunctionPtr(); // NULL
  FunctionPtr fPtr;
  
  assert (i <= myAsl_->i.n_con_);
  cName = std::string(con_name_ASL(myAsl_, i));
  addLinearTermsFromConstr_(lfPtr, i, myAsl_, vars_);
  fPtr = (FunctionPtr) new Function(lfPtr);

  instance->newConstraint(fPtr, myAsl_->i.LUrhs_[2*i], 
      myAsl_->i.LUrhs_[2*i+1], cName); 
}

void addLinearObjective_(int i, ProblemPtr instance, ASL *myAsl_, std::vector<VariablePtr> & vars_)
{
  FunctionPtr fPtr;
  LinearFunctionPtr lfPtr = LinearFunctionPtr(); // null
  ObjectiveType obj_sense;
  std::string oName;

  assert (i <= myAsl_->i.n_obj_);

  if (myAsl_->i.objtype_[i] == 1) {
    obj_sense = Maximize;
  } else {
    obj_sense = Minimize;
  }

  addLinearTermsFromObj_(lfPtr, i, myAsl_, vars_);

  oName = std::string(obj_name_ASL(myAsl_, i));
  fPtr = (FunctionPtr) new Function(lfPtr);
  instance->newObjective(fPtr, objconst_ASL(myAsl_, 0), 
      obj_sense, oName);
}


void addLinearTermsFromConstr_(LinearFunctionPtr & lf, int i, ASL *myAsl_, std::vector<VariablePtr> & vars_)
{
  cgrad *cg; // for ampl
  if (lf) {
    for (cg = myAsl_->i.Cgrad_[i]; cg; cg = cg->next) {
      lf->incTerm(vars_[cg->varno], cg->coef);
    }
  } else {
    lf= (LinearFunctionPtr) new LinearFunction();
    for (cg = myAsl_->i.Cgrad_[i]; cg; cg = cg->next) {
      lf->addTerm(vars_[cg->varno], cg->coef);
    }
  }
  if (lf->getNumTerms()==0) {
    lf.reset();
  }
}

void addLinearTermsFromObj_(LinearFunctionPtr & lf, int i, ASL *myAsl_, std::vector<VariablePtr> & vars_)
{
  ograd *og; // for ampl
  if (lf) {
    for (og = myAsl_->i.Ograd_[i]; og; og = og->next) {
      lf->incTerm(vars_[og->varno], og->coef);
    }
  } else {
    lf= (LinearFunctionPtr) new LinearFunction();
    for (og = myAsl_->i.Ograd_[i]; og; og = og->next) {
      lf->addTerm(vars_[og->varno], og->coef);
    }
  }
  if (lf->getNumTerms()==0) {
    lf.reset();
  }
}

void addPolynomialConstraint_(int i, ProblemPtr instance,
    ASL *myAsl_, std::vector<VariablePtr> & vars_, VariableSet & pvars_, 
    PolyVector & pVec) 
{
    double c = 0;
    FunctionPtr fPtr = FunctionPtr();  //NULL
    LinearFunctionPtr lfPtr = LinearFunctionPtr();  //NULL
    PolynomialPtr pPtr = PolynomialPtr();  //NULL
    cde *constraint_cde;
    // cast myAsl_ from ASL into ASL_fg
    ASL_fg *asl_fg = (ASL_fg *)myAsl_;

    constraint_cde = asl_fg->I.con_de_+i;
    assert (constraint_cde);
    std::cout << "constraint " << i << std::endl;
    getPolynomial_(pPtr, c, constraint_cde->e, myAsl_, vars_);
    pPtr->createVarSet();
    if (myAsl_->i.LUrhs_[2*i] - c > -1e10 && myAsl_->i.LUrhs_[2*i+1] - c < 1e10) {
      assert(!"can not handle ranges/equalities in polynomial constraints");
    } else if (!(myAsl_->i.LUrhs_[2*i] - c > 0 || myAsl_->i.LUrhs_[2*i+1] - c < 0)) {
      std::cout << myAsl_->i.LUrhs_[2*i] - c << " " <<  myAsl_->i.LUrhs_[2*i+1] - c 
        << std::endl;
      assert(!"can not handle given constraint");
    }

    if (myAsl_->i.LUrhs_[2*i] - c > 0) {
      pPtr->setRhs(myAsl_->i.LUrhs_[2*i] - c);
    } else {
      pPtr->setRhs(myAsl_->i.LUrhs_[2*i+1] - c);
    }

    //pPtr->write(std::cout);
    //std::cout << std::endl;
    for (std::vector<ConstVariablePtr>::const_iterator iter=pPtr->varsBegin(); 
        iter!=pPtr->varsEnd(); iter++) {
      pvars_.insert(*iter);
    }
    pVec.push_back(pPtr);
    addLinearTermsFromConstr_(lfPtr, i, myAsl_, vars_);
    assert(!lfPtr);
}

void getPolynomial_(PolynomialPtr & pPtr, double & c, expr *e_ptr, ASL *myAsl_, std::vector<VariablePtr> & vars_)
{
  int opcode;
  c = 0;
  int var_index;
  std::map <efunc*, int> functionMap_;
  PolynomialPtr pPtr1 = PolynomialPtr();  //NULL 
  PolynomialPtr pPtr2 = PolynomialPtr();  //NULL
  double c1=0, c2=0;

  for (int i=0; i<N_OPS; i++) {
    functionMap_[r_ops_ASL[i]] = i;
  }

  opcode = functionMap_[e_ptr->op];
  switch (opcode) {
   case (OPPLUS):   // expr1 + expr2
     getPolynomial_(pPtr1, c1, e_ptr->L.e, myAsl_, vars_);
     getPolynomial_(pPtr2, c2, e_ptr->R.e, myAsl_, vars_);
     pPtr = pPtr2->clone();
     (*pPtr) += pPtr1;
     break;
   case (OPMINUS):  // expr1 - expr2
     getPolynomial_(pPtr1, c1, e_ptr->L.e, myAsl_, vars_);
     getPolynomial_(pPtr2, c2, e_ptr->R.e, myAsl_, vars_);
     pPtr = -1.0*pPtr2;
     (*pPtr) += pPtr1;
     break;
   case (OPMULT): // expr1*expr2
     getPolynomial_(pPtr1, c1, e_ptr->L.e, myAsl_, vars_);
     getPolynomial_(pPtr2, c2, e_ptr->R.e, myAsl_, vars_);
     if (pPtr1 && pPtr2) {
       if (pPtr1->getNumTerms() > 1 || pPtr2->getNumTerms() > 1) {
         assert(!"can't multiply!");
       }
       pPtr = pPtr1->clone();
       (*pPtr) *= pPtr2;
     } else if (pPtr1 && fabs(c2) > 0) {
       pPtr = pPtr1->clone();
       (*pPtr) *= c2;
     } else if (pPtr2 && fabs(c1) > 0) {
       pPtr = pPtr2->clone();
       (*pPtr) *= c1;
     } else {
       assert (!"can't figure out OPMULT!");
     }
     break;
   case (OPPOW): // expr1 ^ expr2
     break;
   case (OPSUMLIST):
     expr **ep, **epe;
     ep = e_ptr->L.ep;
     epe = e_ptr->R.ep;
     pPtr = (PolynomialPtr) new Polynomial();
     while (ep < epe) {
       getPolynomial_(pPtr1, c2, *ep, myAsl_, vars_);
       (*pPtr) += pPtr1;
       c += c2;
       ep++;
     }
     break;
   case (OP1POW): //  OPPOW for R = numeric constant
     c1 = ((expr_n *)e_ptr->R.e)->v;
     getPolynomial_(pPtr1, c2, e_ptr->L.e, myAsl_, vars_);
     pPtr = (*pPtr1).clone();
     pPtr->raiseTo(c1);
     break;
   case (OP2POW): //  expr^2
     getPolynomial_(pPtr1, c1, e_ptr->L.e, myAsl_, vars_);
     if (pPtr1->getNumTerms()>1) {
       assert(!"can't square a polynomial yet!");
     }
     pPtr = pPtr1->clone();
     pPtr->raiseTo(2.0);
     break;
   case (OPUMINUS):
     getPolynomial_(pPtr1, c1, e_ptr->L.e, myAsl_, vars_);
     pPtr = -1.0 * pPtr1;
     break;
   case (OPNUM): //  numeric constant
     c = ((expr_n *)e_ptr)->v;
     //logger_->MsgStream(LogNone) << "constant = " <<  c  << std::endl;
     break;
   case (OPVARVAL): //  single variable
     // not sure if this var_index is correct. no documentation available!
     var_index = (expr_v *)e_ptr - ((ASL_fg *)myAsl_)->I.var_e_;
     pPtr = (PolynomialPtr) new Polynomial(vars_[var_index]);
     //logger_->MsgStream(LogNone) <<  "x" << var_index << std::endl;
     break;
   default:
     assert(!"something else\n");
  }

}

bool should_solve(UInt mybit, PolyVector & pVec)
{
  // visit each polynomial and check if it will have an even number of
  // negative factors.
  for (PolyVectorIterator iter=pVec.begin(); iter!=pVec.end(); iter++) {
    UInt num_neg = 0;
    for (UInt i=0; i<(*iter)->getNumFactors(); i++) {
      if (mybit%2 == 1) {
        num_neg++;
      }
      mybit = mybit/2;
    }
    if (num_neg%2 == 1) {
      return false;
    }
  }
  return true;
}

double solve_nlp(UInt mybit, ProblemPtr inst, PolyVector & pVec)
{
  double obj = INFINITY;
  ProblemPtr new_inst = createProblem(mybit, inst, pVec);

  //new_inst->write(std::cout);
  new_inst->setQuadraticJacobian();
  new_inst->setQuadraticHessian();
  //newInst->setInitialPoint(ix);
  IpoptEnginePtr ipopt_e = (IpoptEnginePtr) new IpoptEngine();
  ipopt_e->load(new_inst);

  //solve
  ipopt_e->solve();
  //std::cout << "status = " << ipopt_e->getStatus() << std::endl;
  if (ipopt_e->getStatus() == 6) {
    //new_inst->write(std::cout);
  }

  if (ipopt_e->getStatus() == ProvenLocalOptimal) {
    obj = ipopt_e->getSolutionValue();
    //std::cout << "obj =  " << obj << std::endl;
    //const double *x = ipopt_e->getSolution();
    //for (UInt i=0; i<inst->getNumVars(); i++) {
    //  std::cout << x[i] << std::endl;
    //}
    //new_inst->write(std::cout);
  }
  return obj;
}

ProblemPtr createProblem(UInt mybit, ProblemPtr inst, PolyVector & pVec)
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

  // add variables
  for (UInt i=0; i<inst->getNumVars(); i++) {
    constVPtr = inst->getVariable(i);
    vCopy = newP->newVariable(constVPtr->getLb(), constVPtr->getUb(), Continuous);
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
    newP->newConstraint(fPtr, constCPtr->getLb(), constCPtr->getUb());
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
  qCopy = QuadraticFunctionPtr(); // NULL
  nlCopy = NonlinearFunctionPtr(); // NULL
  fPtr = (FunctionPtr) new Function(lCopy, qCopy, nlCopy);
  newP->newObjective(fPtr, 0, Minimize, oPtr->getName()); 

  // for each polynomial:
  UInt mybit2 = mybit;
  for (PolyVectorIterator iter=pVec.begin(); iter!=pVec.end(); iter++) {
    // make pairs of aux variables such that each pair has both positive  or
    // both negative variables.
    std::vector<ConstVariablePair> pPairs, nPairs;
    make_pairs(mybit2, (*iter), newP, pPairs, nPairs);
    add_socs(pPairs, nPairs, newP, (*iter)->getRhs());
    for (std::vector<ConstVariablePair>::iterator iter2=pPairs.begin(); iter2!=pPairs.end(); iter2++) {
      ConstVariablePtr v = iter2->first;
      newP->changeBound(v->getId(), 0, INFINITY);
      v = iter2->second;
      if (v->getName().compare("dummy_one")!=0) {
        newP->changeBound(v->getId(), 0, INFINITY);
      }
    }
    for (std::vector<ConstVariablePair>::iterator iter2=nPairs.begin(); iter2!=nPairs.end(); iter2++) {
      ConstVariablePtr v = iter2->first;
      newP->changeBound(v->getId(), -INFINITY, 0);
      v = iter2->second;
      if (v->getName().compare("dummy_one")!=0) {
        newP->changeBound(v->getId(), -INFINITY, 0);
      } else {
        assert(!"dummy variable can not be negated!");
      }
    }
  }

  return newP;
}

void make_pairs(UInt & mybit2, ConstPolynomialPtr pPtr, ProblemPtr newP, 
    std::vector<ConstVariablePair> & pPairs, 
    std::vector<ConstVariablePair> & nPairs)
{
  ConstVariablePtr pos, neg, v;
  std::vector<ConstVariablePtr> pvec, nvec;
  UInt i=0;
  
  // visit each polynomial and check if it will have an even number of
  // positive factors.
  UInt num_pos = 0;
  for (UInt i=0; i<pPtr->getNumFactors(); i++) {
    if (mybit2%2 == 0) {
      num_pos++;
      pvec.push_back(newP->getVariable(pPtr->getAuxVar(i)->getId()));
    } else {
      nvec.push_back(newP->getVariable(pPtr->getAuxVar(i)->getId()));
    }
    mybit2 = mybit2/2;
  }
  if (num_pos%2 == 1) {
    //add a dummy variable.
    v = newP->newVariable(1.0, 1.0, Continuous, "dummy_one");
    pvec.push_back(v);
  }
  assert(nvec.size()%2==0);

  pPairs.clear();
  for (i=0; i<pvec.size(); i+=2) { 
    pPairs.push_back(std::make_pair(pvec[i], pvec[i+1]));
  }

  nPairs.clear();
  for (i=0; i<nvec.size(); i+=2) { 
    nPairs.push_back(std::make_pair(nvec[i], nvec[i+1]));
  }

  //std::cout << "positive pairs = " << pPairs.size() << pvec.size() << std::endl;
  //std::cout << "negative pairs = " << nPairs.size() << nvec.size() << std::endl;
}

void add_socs(std::vector<ConstVariablePair> & pPairs, std::vector<ConstVariablePair> & nPairs, ProblemPtr newP, double rhs)
{
  //std::cout << "size = " << nPairs.size() + pPairs.size() << std::endl;
  assert(nPairs.size() + pPairs.size() > 0);
  // for each pair, add a new variable and a new SOC constraint
  VariablePtr v;
  std::vector<ConstVariablePtr> new_aux_vars;
  std::vector<double> p_const;
  double n_const = 0;
  std::vector<LinearFunctionPtr> p_terms;
  LinearFunctionPtr lf, n_term;
  p_const.push_back(0.0);
  p_const.push_back(0.0);
  for (UInt i=0; i<pPairs.size(); i++) {
    assert(pPairs[i].first);
    assert(pPairs[i].second);
    p_terms.clear();
    if (nPairs.size() + pPairs.size() < 2) {
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(pPairs[i].first, 1.0);
      lf->addTerm(pPairs[i].second, -1.0);
      p_terms.push_back(lf);
      n_term = (LinearFunctionPtr) new LinearFunction();
      n_term->addTerm(pPairs[i].first, 1.0);
      n_term->addTerm(pPairs[i].second, 1.0);
      p_const.clear();
      p_const.push_back(0.0);
      LSOCPtr lsoc = (LSOCPtr) new LSOC(p_terms, n_term, p_const, n_const, 4*rhs);
      newP->newConstraint(lsoc, -INFINITY, 0);
    } else {
      v = newP->newVariable(0, INFINITY, Continuous);
      new_aux_vars.push_back(v);
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(v, 2.0);
      p_terms.push_back(lf);
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(pPairs[i].first, 1.0);
      lf->addTerm(pPairs[i].second, -1.0);
      p_terms.push_back(lf);
      n_term = (LinearFunctionPtr) new LinearFunction();
      n_term->addTerm(pPairs[i].first, 1.0);
      n_term->addTerm(pPairs[i].second, 1.0);
      LSOCPtr lsoc = (LSOCPtr) new LSOC(p_terms, n_term, p_const, n_const, 1e-12);
      newP->newConstraint(lsoc, -INFINITY, 0);
    }
  }
  for (UInt i=0; i<nPairs.size(); i++) {
    assert(nPairs[i].first);
    assert(nPairs[i].second);
    v = newP->newVariable(0, INFINITY, Continuous);
    new_aux_vars.push_back(v);
    p_terms.clear();
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(v, 2.0);
    p_terms.push_back(lf);
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(nPairs[i].first, 1.0);
    lf->addTerm(nPairs[i].second, -1.0);
    p_terms.push_back(lf);
    n_term = (LinearFunctionPtr) new LinearFunction();
    n_term->addTerm(nPairs[i].first, -1.0);
    n_term->addTerm(nPairs[i].second, -1.0);
    LSOCPtr lsoc = (LSOCPtr) new LSOC(p_terms, n_term, p_const, n_const, 1e-12);
    newP->newConstraint(lsoc, -INFINITY, 0);
  }
  //std::cout << "number of new_aux_variables = " << new_aux_vars.size() << std::endl;
  std::vector<ConstVariablePair> new_pPairs, new_nPairs;
  new_nPairs.clear();
  new_pPairs.clear();
  v = VariablePtr(); //NULL
  for (UInt i=0; i<new_aux_vars.size(); i+=2) {
    if (i==new_aux_vars.size()-1) {
      // add a new dummy variable to the instance
      v = newP->newVariable(1.0, 1.0, Continuous, "dummy_one");
      new_pPairs.push_back(std::make_pair(new_aux_vars[i], v));
    } else {
      new_pPairs.push_back(std::make_pair(new_aux_vars[i], new_aux_vars[i+1]));
    }
  }
  if (new_aux_vars.size()>0) {
    // aux_vars is never used. so we dont update it.
    add_socs(new_pPairs, new_nPairs, newP, sqrt(rhs));
  }
}


