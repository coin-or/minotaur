// MINOTAUR -- It's only 1/2 bull
// 
// (C)opyright 2008 - 2014 The MINOTAUR Team.
// 

/**
 * \file TransSep.cpp
 * \brief Declare TransSep class for detecting nonlinear function 
 * separability. It checks whether the given nonlinear objective and
 * constraint functions are separable. If separable, then it reformulates
 *  the original problem.
 * \author Meenarli Sharma, Indian Institute of Technology Bombay. 
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"

#include "Environment.h"
#include "CNode.h"
#include "Logger.h"
#include "Option.h"
#include "Problem.h"
#include "Constraint.h"
#include "Function.h"
#include "NonlinearFunction.h"
#include "LinearFunction.h"
#include "TransSep.h"
#include "CGraph.h"
#include "CNode.h"
#include "OpCode.h"
#include "Objective.h"

//#define SPEW 1

using namespace Minotaur;
const std::string TransSep::me_ = "TransSep: ";

  TransSep::TransSep()
:env_(EnvPtr()), problem_(ProblemPtr()), coeff_(0.0), itnum_(0), sepNodes_(0),
  sepOps_(0), sepVars_(0), sepConst_(0), sepConId_(0), newCons_(0), newVars_(0),
  objSep_(false), sepStatus_(false),lf_(LinearFunctionPtr()), ub_(0), lb_(0),
  f_(0), snodes_(0), sops_(0), svars_(0), sconst_(0), on_(), onop_(),
  nnode_(0), nvar_(0)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


  TransSep::TransSep(EnvPtr env, ProblemPtr problem)
:env_(env), problem_(problem), coeff_(0.0), itnum_(0), sepNodes_(0),
  sepOps_(0), sepVars_(0), sepConst_(0), sepConId_(0),newCons_(0), newVars_(0),
  objSep_(false), sepStatus_(false), lf_(LinearFunctionPtr()), ub_(0), lb_(0),
  f_(0), snodes_(0), sops_(0), svars_(0), sconst_(0), on_(), onop_(),
  nnode_(0), nvar_(0)
{
  logger_   = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
                                     findInt("separability_log_level")->
                                     getValue());
}


TransSep::~TransSep()
{
  problem_.reset();
  env_.reset();
  logger_.reset();
}


void TransSep::findSep() 
{
  candCons();
  objSepCheck();

  // Reset initial points after separability detection
  if (sepConId_.size() > 0 || objSep_ == true) {
    problem_->resetInitialPoint(newVars_);
  }
  disProb();

#if SPEW
  logger_->msgStream(LogDebug) << me_ <<"No. of separable constraints: "
    << sepConId_.size() << std::endl;

  logger_->msgStream(LogDebug) << me_ <<"Is objective seperable: "<<objSep_
    << std::endl;
#endif
}


void TransSep::candCons()
{
  bool sepStatus;
  ConstraintPtr c_ptr;
  FunctionPtr f;
  LinearFunctionPtr lf1;
  CGraphPtr cgp;

  std::vector<CGraphPtr > cg;
  std::vector<UInt > nsp; // No. of separable parts in separable constraints

  // eqs is 0 if cons is of form lb <= f(x), 1 if f(x) <= ub
  std::vector<UInt > eqs;

  CNodeQ dq; 
  ConstVariablePtr v;
  std::vector<LinearFunctionPtr > lfnew; //Linear functions of separable cons
  std::vector<LinearFunctionPtr > lfmod; //modified linear fun of cons
  std::vector<CGraphPtr > nlfnew; //Linear function of new linear cons
  std::vector<double > ubMlf; //ub of modified linear cons
  std::vector<double > lbMlf; //lb of modified linear cons

  for(ConstraintConstIterator c_iter=problem_->consBegin();
      c_iter!=problem_->consEnd(); ++c_iter) {
    c_ptr=*c_iter;
    f=c_ptr->getFunction();
    cgp = boost::dynamic_pointer_cast <CGraph> (f->getNonlinearFunction());

    if (cgp == NULL) {
      sepStatus = false;
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "Constraint: "<< c_ptr->getName() 
        << " does not have cgraph."<< std::endl;
#endif
    } else {
      f_ = true;
      coeff_ = 1.0;
      ub_ = c_ptr->getUb();
      lb_ = c_ptr->getLb();
      // Perform separability check only if constraint is
      // not in equality form
      if (ub_ == INFINITY && lb_ != -INFINITY) {
        eqs.push_back(0);
        if (c_ptr->getLinearFunction()) {
          lf_ = c_ptr->getLinearFunction()->clone();
          //lf_->write(std::cout);
          std::cout << std::endl;
        } else {
          lf_ = (LinearFunctionPtr) new LinearFunction();
        }
        //std::cout << std::endl;
        //std::cout << "**********Constraint:*********** " <<
        //std::endl;
        //c_ptr->write(std::cout); 
        //std::cout << std::endl;

        //std::cout << "---has cgraph---" <<
        //std::endl;
        //cgp->write(std::cout);
        //std::cout << std::endl;
        nvar_ = f->getNonlinearFunction()->numVars();
        nnode_ = cgp->getNumNodes();
        const CNode *o = cgp->getOut();
        sepStatus = outCheck(o);

      } else if (ub_ != INFINITY && lb_ == -INFINITY) { 
        eqs.push_back(1);
        if (c_ptr->getLinearFunction()) {
          lf_ = c_ptr->getLinearFunction()->clone();
          //lf_->write(std::cout);
          std::cout << std::endl;
        } else {
          lf_ = (LinearFunctionPtr) new LinearFunction();
        }
        //std::cout << std::endl;
        //std::cout << "**********Constraint:*********** " <<
        //std::endl;
        //c_ptr->write(std::cout); 
        //std::cout << std::endl;

        //std::cout << "---has cgraph---" <<
        //std::endl;
        //cgp->write(std::cout);
        //std::cout << std::endl;
        nvar_ = f->getNonlinearFunction()->numVars();
        nnode_ = cgp->getNumNodes();
        const CNode *o = cgp->getOut();
        sepStatus = outCheck(o);
      } else {
        sepStatus = false;
      }
    }
    //If constraint is separable mark it for deletion from the original
    //problem. Add a linear constraint corresponding the nonlinear
    //constraints 
    //and add constraints corresponding to the separable parts.

    if (sepStatus == true ) {
      if(sepNodes_.size()>1)  {
        v = (VariablePtr) new Variable();
        //Generating cgraph of separable parts
        //dq contains all the dependent nodes
        //of the cgp
        dq = cgp->dNodes(); 
        //vector cg contains cgraph of separable parts.
        cg = sepCGraph(&dq);
        //no. of separable parts in the constraint  
        nsp.push_back(cg.size());
#if SPEW
        logger_->msgStream(LogDebug2) << me_
          <<"No. of separable parts in constraint:"
          << c_ptr->getName() << " is:"<< cg.size()
          << std::endl;
#endif
        for(UInt i=0; i < cg.size(); i++) {
          v = problem_->newVariable(VarTran);
          lf1 = (LinearFunctionPtr) new LinearFunction();
          lf_->addTerm(v,1);
          lf1->addTerm(v, -1);
          nlfnew.push_back(cg[i]);
          lfnew.push_back(lf1);
          v.reset();
          lf1.reset();
        }
        newVars_ = newVars_ + cg.size();
        newCons_ = newCons_ + cg.size();
        cg.clear();
        sepConId_.push_back(c_ptr->getId());
        ubMlf.push_back(ub_);
        lbMlf.push_back(lb_);
        lfmod.push_back(lf_);
        lf_.reset();
        //Marking the original nonlinear constraint for deletion
        problem_->markDelete(c_ptr);
      } else {
#if SPEW
        logger_->msgStream(LogDebug2) << me_ <<"Constraint: "<< c_ptr->getName()
          << " has become linear"<< std::endl;
#endif     
        ubMlf.push_back(ub_);
        lbMlf.push_back(lb_);
        lfmod.push_back(lf_);
        lf_.reset();
        //Marking the original nonlinear constraint for deletion
        problem_->markDelete(c_ptr);
      }
    } else {
      logger_->msgStream(LogDebug2) << me_ << "Constraint: "<< c_ptr->getName() 
        << " is not separable."<< std::endl;
    }
  } 

  if (sepConId_.size() >0 || lfmod.size() >0) {
    //Removing constraints marked as delete from problem
    problem_->delMarkedCons();

    //Adding the modified linear constraints to the problem
    for(UInt i=0; i < lfmod.size(); i++) {
      f = (FunctionPtr) new Function(lfmod[i]);
      problem_->newConstraint(f, lbMlf[i], ubMlf[i]);
      f.reset();
    }
    //Adding new constraints corresponding to separable parts
    nvar_ = 0; //reusing nvar for counting
    nnode_ = 0;
    for (UInt j=0; j < sepConId_.size() ; j++) {
      nnode_ = nvar_ + nsp[j];
      for (UInt i = nvar_; i < nnode_; i++) {
        if (eqs[j] == 0) {
          f = (FunctionPtr) new Function(lfnew[i], nlfnew[i] );
          problem_->newConstraint(f, 0.0, INFINITY);
          f.reset();
          nvar_++;
        } else { //(eqs[j] == 1) 
          f = (FunctionPtr) new Function(lfnew[i], nlfnew[i] );
          problem_->newConstraint(f, -INFINITY, 0.0);
          f.reset();
          nvar_++;
        }
      } 
    }
    lfmod.clear();
    lfnew.clear();
    nlfnew.clear();
    ubMlf.clear();
    lbMlf.clear();
    nsp.clear();
  }
}

void TransSep::objSepCheck()
{
  bool sepStatus;
  ConstraintPtr c_ptr;
  FunctionPtr f;
  LinearFunctionPtr lf1;
  CGraphPtr cgp;
  std::vector<CGraphPtr > cg; 
  std::vector<UInt > nsp;//No. of separable parts in each separable constraints
  CNodeQ dq; 
  ConstVariablePtr v;
  ObjectivePtr obj;
  obj = problem_->getObjective();
  f = obj->getFunction();
  cgp = boost::dynamic_pointer_cast <CGraph> (f->getNonlinearFunction());
  if (cgp == NULL) {
    sepStatus = false;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "Objective: "<< obj->getName()
      << " does not have cgraph."<< std::endl;
#endif
  } else {
    f_ = false;
    coeff_ = 1.0;
    ub_ = 0.0;
    if ( obj->getLinearFunction()) {
      lf_ = obj->getLinearFunction(); 
    } else {
      lf_ = (LinearFunctionPtr) new LinearFunction();
    }
    nvar_ = f->getNonlinearFunction()->numVars();
    nnode_ = cgp->getNumNodes();
    const CNode *o = cgp->getOut();
    sepStatus = outCheck(o);
  }
  if (sepStatus == true ) {
    if(sepNodes_.size()>1)  {
      objSep_=true;
      //Generating cgraph of separable parts
      //dq contains all the dependent nodes of the cgp
      dq = cgp->dNodes(); 
      //cg contains cgraph of separable parts.
      cg = sepCGraph(&dq);
      nsp.push_back(cg.size());

      v = (VariablePtr) new Variable();
      for(UInt i=0; i < cg.size(); i++) {
        v = problem_->newVariable(VarTran);
        lf1 = (LinearFunctionPtr) new LinearFunction();
        newVars_++;
        lf_->addTerm(v,1);
        lf1->addTerm(v, -1);
        f = (FunctionPtr) new Function(lf1, cg[i]);
        //Adding separable constraints to the problem
        problem_->newConstraint(f, -INFINITY, 0.0);
        newCons_++;
        v.reset();
        lf1.reset();
        f.reset();
      }
      cg.clear();
      //Removing old objective and adding a new one
      problem_->removeObjective();
      f = (FunctionPtr) new Function(lf_);
      problem_->newObjective(f,(obj->getConstant()+ub_),obj->getObjectiveType());
      lf_.reset();
    }  else {
#if SPEW
      logger_->msgStream(LogDebug2) << me_ << "Objective "<< obj->getName() 
        << " has become linear."<< std::endl;
#endif  
      //Removing old objective and adding a new one
      problem_->removeObjective();
      f = (FunctionPtr) new Function(lf_);
      problem_->newObjective(f,(obj->getConstant()+ub_),obj->getObjectiveType());
      lf_.reset();
    } 

  } else {
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "Objective is not separable."
      << std::endl;
#endif 
  }
} 

bool TransSep::outCheck(const CNode * o)
{
  CNode * n1=0;

  switch(o->getOp()){
  case (OpPlus):
    on_.push(o->getL());
    onop_.push(1);
    on_.push(o->getR());
    onop_.push(1);
    return  sepCheck();
    break;
  case (OpMinus):
    on_.push(o->getL());
    onop_.push(1);  
    on_.push(o->getR());
    onop_.push(-1); 
    return  sepCheck();
    break;
  case (OpSumList):
    for (CNode **it=o->getListL(); it!=o->getListR(); ++it) {
      n1 = *it;
      on_.push(n1);
      onop_.push(1);
    }  
    return  sepCheck();
    break;
  case (OpUMinus):
    coeff_ = coeff_*(-1);
    return  outCheck(o->getL());
    break;
  case (OpPowK):
    if (o->getR()->getVal() == 1.0) {
      return  outCheck(o->getL());
      break;
    } else {
      return false;
    }
  case (OpIntDiv):
    coeff_ = coeff_ *(1/ (o->getR()->getVal()));
    return  outCheck(o->getL());
    break;
  case (OpAbs):
  case (OpAcos):
  case (OpAcosh):
  case (OpAsin):
  case (OpAsinh):
  case (OpAtan):
  case (OpAtanh):
  case (OpCeil):
  case (OpCos):
  case (OpCosh):
  case (OpCPow):
  case (OpExp):
  case (OpFloor):
  case (OpLog):
  case (OpLog10):
  case (OpRound):
  case (OpSin):
  case (OpSinh):
  case (OpSqr):
  case (OpTan):
  case (OpTanh):
  case (OpSqrt):
  case (OpNum):
  case (OpInt):
  case (OpVar):
    return false;
    break;
  case (OpDiv):
    if (o->getR()->getOp() == OpNum || o->getR()->getOp() == OpInt) {
      coeff_ = coeff_ *(1/ (o->getR()->getVal()));
      return  outCheck(o->getL());
    } else {
      return false;    
    }
    break;
  case (OpMult):
    if (o->getR()->getOp() == OpNum || o->getR()->getOp() == OpInt) {
      coeff_ =coeff_ * (o->getR()->getVal());
      return  outCheck(o->getL());
    } else if (o->getL()->getOp() == OpNum || o->getL()->getOp() == OpInt) {
      coeff_ =coeff_*( o->getL()->getVal());
      return  outCheck(o->getR());
    } else {
      return false;    
    }
    break;
  case (OpPow):
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "Opcode OpPow is not implemented."
      << std::endl;
#endif 
    assert(!"not implemented!");
    return false;
    break;
  case (OpNone):
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "Opcode OpNone is found."
      << std::endl;
#endif
    assert(!"warning: encountered node with opcode OpNode.");
    return false;
    break;
  default:
#if SPEW
    logger_->msgStream(LogDebug2) << me_ << "Opcode not is not known."
      << std::endl;
#endif
    assert(!"cannot evaluate!");
    return false;
    break;
  }
  return false;
}


std::vector<CGraphPtr> TransSep::sepCGraph(CNodeQ * dq)
{
  for(UInt i=0; i < nnode_; i++) {
    itnum_.push_back(0);
    tempN_.push_back(NULL);
  }

  CNode * n1;
  std::vector<CGraphPtr> cg;
  UInt k=0;
  for(UInt i=0; i < sepNodes_.size(); ) {
    CGraphPtr t =(CGraphPtr) new CGraph();
    for (UInt j=0; j < sepVars_[i].size(); j++) {
      k = sepVars_[i][j]->getIndex();
      tempN_[k] = t->newNode(problem_->
                             getVariable(sepVars_[i][j]->getV()->getId()));
      itnum_[k]= i+1; //n1 is in separable part i+1
    }
    //Adding constant nodes to the cgraph directly
    for (UInt j=0; j < sepConst_[i].size(); j++) {
      n1 = sepConst_[i][j];
      k = n1->getIndex();
      tempN_[k] = t->newNode(n1->getLb());
      itnum_[k]= i+1; 
    } 
    //Marking the dependent nodes as to which separable part they belong.
    markVis(i);
    cg.push_back(t);
    t.reset();
    i++;
  }
  sepConst_.clear();
  createCG(&cg, dq);
  finalCG(&cg);
  return cg;
}

void TransSep::createCG(std::vector<CGraphPtr > * cg, CNodeQ * dq)
{
  //generating cgraph of separable parts
  CNode **childr = 0;
  UInt k=0, c2=0, f1;
  CNode *n1, *nl =0;
  for (UInt i=0; i < (*dq).size(); i++) {
    n1 = (*dq)[i];
    k = itnum_[n1->getIndex()] ;
    if (k > 0) { 
      switch(n1->getOp()) {
      case (OpAbs):
      case (OpAcos):
      case (OpAcosh):
      case (OpAsin):
      case (OpAsinh):
      case (OpAtan):
      case (OpAtanh):
      case (OpCeil):
      case (OpCos):
      case (OpCosh):
      case (OpCPow):
      case (OpExp):
      case (OpFloor):
      case (OpLog):
      case (OpLog10):
      case (OpRound):
      case (OpSin):
      case (OpSinh):
      case (OpSqr):
      case (OpTan):
      case (OpTanh):
      case (OpUMinus):
      case (OpSqrt):
        popuTempN((*cg)[k-1], n1, n1->getOp(),0);
        break;
      case (OpDiv):
      case (OpPowK):
      case (OpIntDiv):
      case (OpPlus):
      case (OpMult):
      case (OpMinus):
        popuTempN((*cg)[k-1], n1, n1->getOp(),1);
        break;
      case (OpNone):
        assert(!"warning: encountered node with opcode OpNode.");
        break; 
      case (OpNum):
      case (OpInt):
      case (OpVar):
        break;
      case (OpPow):
        assert(!"not implemented!");
        break;
      case (OpSumList):
        f1 = n1->numChild();
        childr = new Minotaur::CNode *[f1];
        c2=0;
        for (CNode **it=n1->getListL(); it!=n1->getListR(); ++it) {
          nl = *it;
          childr[c2]=tempN_[nl->getIndex()];
          c2++;
        }
        tempN_[n1->getIndex()]=(*cg)[k-1]->
          newNode(Minotaur::OpSumList,childr,f1);
        delete []childr;
        break;
      default:
        assert(!"cannot evaluate!");
      }
    }
  }
  itnum_.clear();
  (*dq).clear();
}


void TransSep::popuTempN(CGraphPtr c, CNode *n1, OpCode op, bool k)
{
  CNode * nl;
  if (k) {
    CNode * nr;
    nl = getLchild(n1);
    nr = getRchild(n1);
    tempN_[n1->getIndex()]= c->newNode(op, nl, nr);   
  } else {
    nl = getLchild(n1);
    tempN_[n1->getIndex()] = c->newNode(op, nl, 0);          
  }
}


void TransSep::finalCG(std::vector<CGraphPtr > * cg)
{
  UInt c2=0, c3=0, f1;
  CNode **childr = 0;
  CNode * n1=0, *nl =0, *n2=0, *nr=0;
  for (UInt i=0; i < sepNodes_.size(); i++) {
    f1 = sepNodes_[i].size();
    if (f1 == 1) {
      if (sepOps_[i][0] == 1) {
        nl = sepNodes_[i][0];
        n1 = tempN_[nl->getIndex()];
      } else {
        nl = tempN_[sepNodes_[i][0]->getIndex()];
        n1 = (*cg)[i]->newNode(Minotaur::OpUMinus, nl, 0);
      }
    } else if (f1 == 2) {
      if (sepOps_[i][0] == sepOps_[i][1] && sepOps_[i][0] == 1) {
        c2 = sepNodes_[i][0]->getIndex();
        nl = tempN_[c2];
        c2 = sepNodes_[i][1]->getIndex();
        nr = tempN_[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpPlus, nl, nr);
      }
      if (sepOps_[i][0] == sepOps_[i][1] && sepOps_[i][0] == -1) {
        c2 = sepNodes_[i][0]->getIndex();
        n2 = tempN_[c2];
        nl = (*cg)[i]->newNode(Minotaur::OpUMinus, n2, 0);
        c2 = sepNodes_[i][1]->getIndex();
        n2 = tempN_[c2];
        nr = (*cg)[i]->newNode(Minotaur::OpUMinus, n2, 0);
        n1 = (*cg)[i]->newNode(Minotaur::OpPlus, nl, nr); 
      }
      if (sepOps_[i][0] != sepOps_[i][1] && sepOps_[i][0] == 1) {
        c2 = sepNodes_[i][0]->getIndex();
        nl = tempN_[c2];
        c2 = sepNodes_[i][1]->getIndex();
        nr = tempN_[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr);
      }
      if (sepOps_[i][0] != sepOps_[i][1] && sepOps_[i][0] == -1) {
        c2 = sepNodes_[i][0]->getIndex();
        nr= tempN_[c2];
        c2 = sepNodes_[i][1]->getIndex();
        nl = tempN_[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr);       
      }
    } else { // (f1 > 2) 
      childr = new Minotaur::CNode *[f1];
      c2=0;
      for(UInt j=0; j<sepNodes_[i].size(); j++) {
        if (sepOps_[i][j] == 1) {
          c3 = sepNodes_[i][j]->getIndex();
          childr[c2]=tempN_[c3];
          c2++;     
        } else {
          c3 = sepNodes_[i][j]->getIndex();
          n1 = tempN_[c3];
          childr[c2]=(*cg)[i]->newNode(Minotaur::OpUMinus, n1, 0);     
          c2++;
        }
      }
      n1 = (*cg)[i]->newNode(Minotaur::OpSumList, childr, f1);          
      delete []childr;
    }
    if (coeff_ != 1.0) {
      nl = (*cg)[i]->newNode(coeff_);          
      nr = n1;
      n1 = (*cg)[i]->newNode(Minotaur::OpMult, nl, nr);          
    } 
    (*cg)[i]->setOut(n1);
    (*cg)[i]->finalize();
  }
  sepNodes_.clear();
  sepVars_.clear();
  sepOps_.clear();
  tempN_.clear();
}

void TransSep::markVis(UInt i)
{
  CNode *n1=0, *n2=0;
  std::stack<CNode * > ts;
  for(UInt j=0; j<sepNodes_[i].size(); j++) {
    n1 = sepNodes_[i][j];
    ts.push(n1);
    while (ts.size() != 0) {
      n1 = ts.top();
      switch(n1->getOp()){
      case (OpAbs):
      case (OpAcos):
      case (OpAcosh):
      case (OpAsin):
      case (OpAsinh):
      case (OpAtan):
      case (OpAtanh):
      case (OpCeil):
      case (OpCos):
      case (OpCosh):
      case (OpCPow):
      case (OpExp):
      case (OpFloor):
      case (OpLog):
      case (OpLog10):
      case (OpRound):
      case (OpSin):
      case (OpSinh):
      case (OpSqr):
      case (OpTan):
      case (OpTanh):
      case (OpUMinus):
      case (OpSqrt):
        itnum_[n1->getIndex()] = i+1;
        ts.pop();
        if (itnum_[n1->getL()->getIndex()] == 0) { 
          ts.push(n1->getL());
        }
        break;
      case (OpDiv):
      case (OpPlus):
      case (OpMult):
      case (OpMinus):
        itnum_[n1->getIndex()] = i+1;
        ts.pop();
        if (itnum_[n1->getL()->getIndex()] == 0) { 
          ts.push(n1->getL());
        }
        if (itnum_[n1->getR()->getIndex()] == 0) { 
          ts.push(n1->getR());
        }
        break;
      case (OpPowK):
      case (OpIntDiv):
        itnum_[n1->getIndex()] = i+1;
        ts.pop();
        if (itnum_[n1->getL()->getIndex()] == 0) { 
          ts.push(n1->getL());
        }
        ts.push(n1->getR());
        break;
      case (OpNone):
        assert(!"warning: encountered node with opcode OpNone");
        break; 
      case (OpNum):
      case (OpInt):
      case (OpVar):
        ts.pop();
        break;
      case (OpPow):
        assert(!"not implemented!");
        break;
      case (OpSumList):
        itnum_[n1->getIndex()] = i+1;
        ts.pop();
        for (CNode **it=n1->getListL(); it!=n1->getListR(); ++it) {
          n2 = *it;
          if (itnum_[n2->getIndex()] == 0) { 
            ts.push(n2);
          }
        }           
        break;
      default:
        assert(!"cannot evaluate!");
      }
    }
  }
}

void TransSep::clearCont()
{
  snodes_.clear();
  sops_.clear();
  svars_.clear();
  sconst_.clear();
  itnum_.clear(); 
  sepNodes_.clear();
  sepOps_.clear();
  sepVars_.clear();
  sepConst_.clear();
  on_=  std::stack<CNode *> ();
  onop_ = std::stack<int > (); 
}

void TransSep::populate()
{
  sepNodes_.push_back(snodes_) ;
  sepOps_.push_back(sops_) ;
  sepVars_.push_back(svars_) ;
  sepConst_.push_back(sconst_) ;

  snodes_.clear();
  sops_.clear();
  svars_.clear();
  sconst_.clear();
}


void TransSep::updateLin(VariablePtr v, double d)
{
  if (lf_->hasVar(v) == true) {
    lf_->incTerm(v, d);
  } else {
    lf_->addTerm(v, d);
  }
  on_.pop();
  onop_.pop();
}


void TransSep::popuon(CNode * n1, int opc, int t)
  //t for taking into account opcode sign
{
  on_.pop();
  onop_.pop();
  on_.push(n1->getL());
  onop_.push(opc);
  on_.push(n1->getR());
  onop_.push(opc*t);
}

bool TransSep::sepCheck()
{
  CNode *n1 = 0, *n2 =0;
  int opc;
  UInt  j=0; //j is iteration counter
  std::vector<UInt > m;  //Variable m is used in merge operation
  m.push_back(0);
  for(UInt i=0; i < nnode_; i++) {
    itnum_.push_back(0);
  }
  while (on_.size() != 0) {
    n1=on_.top();
    opc=onop_.top();
    switch (n1->getOp()){ 
    case (OpNum):
    case (OpInt):
      {
        // Update lower and upper bounds of the constraint
        // or constant term in case of objective
        double d1 = n1->getVal();
        if (f_ == 1) {
          if (d1 > 0) {
            ub_ = ub_ - (opc * coeff_ * d1);
            lb_ = lb_ - (opc * coeff_ * d1); 
          } else {
            lb_ = lb_ + (opc * coeff_ * d1); 
            ub_ = ub_ + (opc * coeff_ * d1);
          }
        } else {
          ub_ = ub_ + (opc * coeff_ * d1);
        }
        on_.pop();
        onop_.pop();
        break;
      }
    case (OpVar):
      //Update the linear part of the function
      {
        VariablePtr v;
        v = problem_->getVariable(n1->getV()->getId());
        updateLin(v, opc*coeff_);
        break;
      }
    case (OpPlus):
      if (itnum_[n1->getIndex()]==0) {
        popuon(n1, opc, 1);
      } else {
        bool sep = visited(&j, &m, n1, opc);
        if (!sep)
          return false;        
      }
      break;
    case (OpMinus):
      if (itnum_[n1->getIndex()]==0) {
        popuon(n1, opc, -1);
      } else {
        bool sep = visited(&j, &m, n1, opc);
        if (!sep)
          return false;        
      }
      break;
    case (OpSumList):
      if (itnum_[n1->getIndex()]==0) {
        on_.pop();
        onop_.pop();
        for (CNode **it=n1->getListL(); it!=n1->getListR(); ++it) {
          n2 = *it;
          on_.push(n2);
          onop_.push(opc);
        } 
      } else {
        bool sep = visited(&j, &m, n1, opc);
        if (!sep)
          return false;        
      }
      break; 
    case (OpUMinus):
      if (itnum_[n1->getIndex()]==0) {
        on_.pop();
        onop_.pop();
        on_.push(n1->getL());
        onop_.push(opc*(-1));
      } else {
        bool sep = visited(&j, &m, n1, opc);
        if (!sep)
          return false;        
      }
      break;
    case (OpAbs):
    case (OpAcos):
    case (OpAsin):
    case (OpAsinh):
    case (OpAtan):
    case (OpAtanh):
    case (OpCeil):
    case (OpCos):
    case (OpCosh):
    case (OpCPow):
    case (OpExp):
    case (OpFloor):
    case (OpLog):
    case (OpLog10):
    case (OpRound):
    case (OpSin):
    case (OpSinh):
    case (OpSqr):
    case (OpTan):
    case (OpTanh):
    case (OpSqrt):
      explore(j, n1, &m, opc);
      clearpopu(&j, &m);
      break;
    case (OpMult):
      if (n1->getL()->getOp() == OpNum || n1->getL()->getOp() == OpInt) {
        if (n1->getR()->getOp() == OpVar){
          VariablePtr v;
          v = problem_->getVariable(n1->getR()->getV()->getId());
          updateLin(v, ((n1->getL()->getVal())*opc*coeff_));
          break;       
        } else {
          explore(j, n1, &m, opc);
        }
      } else if (n1->getR()->getOp() == OpNum || n1->getR()->getOp() == OpInt){
        if (n1->getL()->getOp() == OpVar){
          VariablePtr v;
          v = problem_->getVariable(n1->getL()->getV()->getId());
          updateLin(v, ((n1->getR()->getVal())*opc*coeff_));
          break;       
        } else {
          explore(j, n1, &m, opc);
        }
      } else {
        explore(j, n1, &m, opc);
      }
      clearpopu(&j, &m);
      break;
    case (OpDiv):
      if (n1->getR()->getOp() == OpNum || n1->getR()->getOp() == OpInt){
        if (n1->getL()->getOp() == OpVar){
          VariablePtr v;
          v = problem_->getVariable(n1->getL()->getV()->getId());
          updateLin(v,  (opc*coeff_)/(n1->getR()->getVal()));
          break;       
        } else {
          explore(j, n1, &m, opc);
        }
      } else {
        explore(j, n1, &m, opc);
      }
      clearpopu(&j, &m);
      break;
    case (OpPowK):
      if (n1->getR()->getVal() == 1.0 || n1->getL()->getOp() == OpVar) {
        VariablePtr v;
        v = problem_->getVariable(n1->getL()->getV()->getId());
        updateLin(v, opc*coeff_);
        break;      
      } else {
        explore(j, n1, &m, opc);
      }
      clearpopu(&j, &m);
      break;
    case (OpIntDiv):
      if (n1->getL()->getOp() == OpVar){
        VariablePtr v;
        v = problem_->getVariable(n1->getL()->getV()->getId());
        updateLin(v,  (opc*coeff_)/(n1->getR()->getVal()));
        break;       
      } else {
        explore(j, n1, &m, opc);
      }
      clearpopu(&j, &m);
      break;
    case (OpNone):
      assert(!"warning: encountered a node with opcode OpNone");
      break;
    case (OpPow):
      assert(!"warning: not implemented node with opcode OpPow");
      break;
    default:
      assert(!"cannot evaluate!");
      break;
    }
  }
  itnum_.clear();
  m.clear();

  //Cleaning of sepNodes_ by removing zero entries formed during
  //merge operation
  for(UInt i=0; i < sepNodes_.size(); ) {
    if (sepNodes_[i].size() == 0) {
      sepNodes_.erase(sepNodes_.begin()+i);  
      sepOps_.erase(sepOps_.begin()+i);  
      sepVars_.erase(sepVars_.begin()+i);  
      sepConst_.erase(sepConst_.begin()+i);  
    } else {
      i++;
    }
  }
  //Continue separability detection only if searable parts are
  //more than one.
  if (sepNodes_.size() == 1) {
    logger_->msgStream(LogDebug2) << me_ << "function under consideration" 
      << " has only one nonlinear term."
      << std::endl;
    return false;
  } else
    return true;
}

// Adding the information of the node which is already visited
bool TransSep::visited(UInt* j, std::vector<UInt >*m, CNode *n1,
                       int opc)
{
  UInt mNum; //mNum for merge operation
  snodes_.push_back(n1);
  sops_.push_back(opc);
  mNum=mergeIt(*j, itnum_[n1->getIndex()], m);
  merge(mNum);
  on_.pop();
  onop_.pop();
  return clearpopu(j, m);
}

bool TransSep::clearpopu(UInt *j, std::vector<UInt >*m)
{
  if (svars_.size() == nvar_) {
    clearCont(); 
    return false;  
  }
  populate();
  (*j)++;
  (*m).push_back(0);
  return true;

}

void TransSep::explore (int j, CNode *n1, std::vector<UInt > *m, int opc)
{
  UInt mNum;
  if (itnum_[n1->getIndex()]==0) {
    itnum_[n1->getIndex()]=j+1;
    snodes_.push_back(n1);
    sops_.push_back(opc);
    depthFS(j, n1, m);
    on_.pop();
    onop_.pop();
  } else {
    snodes_.push_back(n1);
    sops_.push_back(opc);
    mNum=mergeIt(j, itnum_[n1->getIndex()], m);
    merge(mNum);
    on_.pop();
    onop_.pop();
  }
}


int TransSep::mergeIt(int j, int a, std::vector<UInt > * m)
{
  int b;
  if ((*m)[a-1] == 0) {
    (*m)[a-1]=j+1;
    return (a-1);
  } else if ((*m)[a-1] == (UInt) j+1) {
    return -1;
  } else {
    b=(*m)[a-1];
    (*m)[a-1]=j+1;
    return mergeIt(j, b, m); 
  }
}


void TransSep::merge(int mNum)
{
  if (sepNodes_[mNum][0]->getIndex() > snodes_[0]->getIndex()) {
    snodes_.push_front(sepNodes_[mNum][0]);
    sops_.push_front(sepOps_[mNum][0]);
    for(UInt i=1; i<sepNodes_[mNum].size(); i++) {
      snodes_.push_back(sepNodes_[mNum][i]);
      sops_.push_back(sepOps_[mNum][i]);
    }
  } else {
    for(UInt i=0; i<sepNodes_[mNum].size(); i++) {
      snodes_.push_back(sepNodes_[mNum][i]);
      sops_.push_back(sepOps_[mNum][i]);
    }
  }

  sepNodes_[mNum].clear();
  sepOps_[mNum].clear();

  for(UInt i=0; i<sepVars_[mNum].size(); i++) {
    svars_.push_back(sepVars_[mNum][i]);
  }
  sepVars_[mNum].clear();

  for(UInt i=0; i<sepConst_[mNum].size(); i++) {
    sconst_.push_back(sepConst_[mNum][i]);
  }
  sepConst_[mNum].clear();

}


void TransSep::depthFS(int j, CNode *n1, std::vector<UInt > *m)
{
  std::stack<CNode *> tempNodes;
  CNode *n=0, *n2=0;
  int mNum;
  tempPop(n1, &tempNodes);
  while (tempNodes.size() != 0) {
    n=tempNodes.top();
    if (itnum_[n->getIndex()] == 0) {
      switch(n->getOp()){
      case (OpAbs):
      case (OpAcos):
      case (OpAcosh):
      case (OpAsin):
      case (OpAsinh):
      case (OpAtan):
      case (OpAtanh):
      case (OpCeil):
      case (OpCos):
      case (OpCosh):
      case (OpCPow):
      case (OpExp):
      case (OpFloor):
      case (OpLog):
      case (OpLog10):
      case (OpRound):
      case (OpSin):
      case (OpSinh):
      case (OpSqr):
      case (OpTan):
      case (OpTanh):
      case (OpUMinus):
      case (OpSqrt):
        tempNodes.pop();
        tempNodes.push(n->getL());
        itnum_[n->getIndex()]=j+1;
        break;
      case (OpDiv):
      case (OpPowK):
      case (OpPlus):
      case (OpMult):
      case (OpMinus):
      case (OpIntDiv):
        tempNodes.pop();
        tempNodes.push(n->getR());
        tempNodes.push(n->getL());
        itnum_[n->getIndex()]=j+1;
        break;
      case (OpNum):
      case (OpInt):
        tempNodes.pop();
        sconst_.push_back(n);
        break;
      case (OpVar):
        tempNodes.pop();
        svars_.push_back(n);
        itnum_[n->getIndex()]=j+1;
        break;
      case (OpPow):
        assert(!"not implemented!");
        break;
      case (OpNone):
        assert(!"warning: encountered node with opcode OpNode.");
        break;
      case (OpSumList):
        tempNodes.pop();
        for (CNode **it=n->getListL(); it!=n->getListR(); ++it) {
          n2 = *it;
          tempNodes.push(n2);
        }
        break;
      default:
        assert(!"cannot evaluate!");
      }

    } else {
      if (itnum_[n->getIndex()] == j+1) {
        tempNodes.pop();      
      } else {
        mNum=mergeIt(j, itnum_[n->getIndex()], m);
        if (mNum > -1) {
          //mNum = -1 if a node is encountered twice in an iteration
          merge(mNum);
        }
        tempNodes.pop();      
      }
    }
  }
}


void TransSep::tempPop(CNode *n1, std::stack<CNode *> * tempNodes)
{
  CNode *n=0;
  switch(n1->getOp()){
  case (OpAbs):
  case (OpAcos):
  case (OpAcosh):
  case (OpAsin):
  case (OpAsinh):
  case (OpAtan):
  case (OpAtanh):
  case (OpCeil):
  case (OpCos):
  case (OpCosh):
  case (OpCPow):
  case (OpExp):
  case (OpFloor):
  case (OpLog):
  case (OpLog10):
  case (OpRound):
  case (OpSin):
  case (OpSinh):
  case (OpSqr):
  case (OpTan):
  case (OpTanh):
  case (OpUMinus):
  case (OpSqrt):
    (*tempNodes).push(n1->getL());
    break;
  case (OpDiv):
  case (OpPowK):
  case (OpPlus):
  case (OpMult):
  case (OpMinus):
  case (OpIntDiv):
    (*tempNodes).push(n1->getR());
    (*tempNodes).push(n1->getL());
    break;
  case (OpNum):
  case (OpInt):
  case (OpVar):
    break;
  case (OpPow):
    assert(!"not implemented!");
    break;
  case (OpNone): 
    assert(!"warning: encountered node with opcode OpNode.");
    break;
  case (OpSumList):
    for (CNode **it=n1->getListL(); it!=n1->getListR(); ++it) {
      n = *it;
      (*tempNodes).push(n);
    }
    break;
  default:
    assert(!"cannot evaluate!");
  }
}

bool TransSep::getStatus() 
{
  if (sepConId_.size() > 0 || objSep_ == true) {
    sepStatus_ = true;  
  } else {
    sepStatus_ = false;  
  }
  return sepStatus_;
}


void TransSep::disProb()
{
  env_->getLogger()->msgStream(LogInfo) << me_ 
    << "Finished separability detection"
    << std::endl;
  problem_->calculateSize(true);
  if (true == env_->getOptions()->findBool("display_separable_size")->
      getValue()) {
    problem_->writeSize(logger_->msgStream(LogNone));
  }
  if (true == env_->getOptions()->findBool("display_separable_problem")->
      getValue()) {
    problem_->write(logger_->msgStream(LogNone));
  }
  env_->getLogger()->msgStream(LogExtraInfo) << me_ 
    << "Is objective seperable: " 
    <<objSep_  << std::endl;
  env_->getLogger()->msgStream(LogExtraInfo) << me_ 
    <<"No. of separable constraints: "
    << sepConId_.size() << std::endl;
  env_->getLogger()->msgStream(LogExtraInfo)<< me_ 
    << "Index of separable constraints:" 
    << std::endl ;
  for(UInt i =0; i < sepConId_.size(); i++) {

    env_->getLogger()->msgStream(LogExtraInfo) << sepConId_[i] << std::endl;
  }
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

