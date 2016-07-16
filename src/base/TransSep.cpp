//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file TransSep.cpp
 * \brief Declare the SeparabilityDectector class for detecting nonlinear
 * function separability. It checks whether the given nonlinear function
 * is separable. If separable, then it reformulates the original problem.
 * It also gives separable parts and computational graph of each separable
 * part.
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
: env_(EnvPtr()), problem_(ProblemPtr()), coeff_(0.0), bcoeff_(false), 
  itnum_(0), SepNodes_(0), SepOps_(0), SepVars_(0), SepConst_(0), sepC_(0), newCons_(0), newVars_(0), objSep_(false)
  //SepOps_(0), SepVars_(0), SepConst_(0), sepC_(0), newCons_(0), newVars_(0), Osep_(0)
  //SepOps_(0), SepVars_(0), SepConst_(0), sepC_(0), newCons_(0), newVars_(0), objSep_(false)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


  TransSep::TransSep(EnvPtr env, ProblemPtr problem)
: env_(env), problem_(problem), coeff_(0.0), bcoeff_(false),
  itnum_(0), SepNodes_(0), SepOps_(0), SepVars_(0), SepConst_(0), sepC_(0), newCons_(0), newVars_(0), objSep_(false)
  //SepOps_(0), SepVars_(0), SepConst_(0), sepC_(0), newCons_(0), newVars_(0), sep_(0)
  //SepOps_(0), SepVars_(0), SepConst_(0), sepC_(0), newCons_(0), newVars_(0), objSep_(false)
{
  logger_   = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
                                     findInt("separability_log_level")->getValue());

  //Check constraints separability
  candCons();
  //checking objective separability
  objSepCheck();
  disProb();
  std::cout << "No. of separable constraints: "<< sepC_ << std::endl;
  std::cout << "Is objective seperable:  "<< objSep_ << std::endl;
}


TransSep::~TransSep()
{
  problem_.reset();
  env_.reset();
  logger_.reset();
}

//Check constraints of the problem one by one. If cgraph exists, check for
// separabiity. Same for objective of the problem.
void TransSep::candCons()
{
  //Flag set to true if constraint is separable. Otherwise false.
  bool sepStatus;
  ConstraintPtr c_ptr;
  FunctionPtr f;
  NonlinearFunctionPtr nlf;
  LinearFunctionPtr lf, lf1;
  //LinearFunctionPtr lf0, lf, lf1;
  CGraphPtr cgp;
  //CNode  *n1=0;

  UInt nvar=0, nnode=0;

  std::vector<CGraphPtr > cg; 
  CNodeQ dq; 
  ConstVariablePtr v;
  std::vector<LinearFunctionPtr > lfnew; //Linear functions of separable cons
  std::vector<LinearFunctionPtr > lfmod; //modified linear fun of cons
  std::vector<CGraphPtr > nlfnew; //Linear function of new linear constraints
  std::vector<double > ubMlf; //ub of modified linear constraints
  std::vector<double > lbMlf; //lb of modified linear constraints
  //std::vector<VariablePtr > newVar; //new variables added for new separable constraints

  for(ConstraintConstIterator c_iter=problem_->consBegin();
      c_iter!=problem_->consEnd(); ++c_iter) {
    c_ptr=*c_iter;


    f=c_ptr->getFunction();
    cgp = boost::dynamic_pointer_cast <CGraph> (f->getNonlinearFunction());
    // assert(cg);
    if (cgp == NULL) {
      sepStatus = false;
#if SPEW
      logger_->msgStream(LogDebug) << me_ << "Constraint: "<< c_ptr->getName() 
        << " does not have cgraph.\n"<< std::endl;
#endif

#if 0
      std::cout << "Constraint: " << std::endl;
      c_ptr->write(std::cout);
      std::cout << " does not have cgraph.\n";
#endif

    } else {
      coeff_ = 1.0;
      bcoeff_ = false;
      //if ( c_ptr->getLinearFunction()) {
        //lf = c_ptr->getLinearFunction()->clone(); 
      //} else {
        //lf = (LinearFunctionPtr) new LinearFunction();
      //}

      nvar = f->getNonlinearFunction()->numVars();
      nnode = cgp->getNumNodes();
      const CNode *o = cgp->getOut();
      sepStatus = outCheck(o, nnode, nvar);

#if 0
      std::cout << "cgraph " << std::endl;
      cgp->write(std::cout);

      std::cout << "Constraint: " << std::endl;
      c_ptr->write(std::cout); 
      if (sepStatus == true) {
        std::cout << " is separable. \n";
      } else {
        std::cout << " is not separable.\n";
      }

#endif
    }
    //If constraint is separable mark it for deletion from the original
    //problem. Add a linear constraint corresponding the nonlinear constraints 
    //and add constraints corresponding to the separable parts.
    if (sepStatus == true ) {
      //Generating cgraph of separable parts
      //dq contains all the dependent nodes of the cgp
      dq = cgp->dNodes(); 
      //This vector contains cgraph of separable parts.
      cg = SepCGraph(nnode, &dq);

      if ( c_ptr->getLinearFunction()) {
        lf = c_ptr->getLinearFunction()->clone(); 
      } else {
        lf = (LinearFunctionPtr) new LinearFunction();
      }


      //std::cout << std::endl;
      //std::cout << "linear function " << std::endl;
      //lf->write(std::cout);
      //std::cout << std::endl;

      //lfmod[sepC_] = (LinearFunctionPtr) new LinearFunction();
      //check if bounds are copied.
      //lfmod[sepC_] = c_ptr->getLinearFunction(); 
      ubMlf.push_back(c_ptr->getUb());
      lbMlf.push_back(c_ptr->getLb());


      v = (VariablePtr) new Variable();
      for(UInt i=0; i < cg.size(); i++) {

        lf1 = (LinearFunctionPtr) new LinearFunction();
        if (c_ptr->getLinearFunction()!= NULL) {
          for (UInt j=0; j< SepVars_[i].size(); j++) {
            v = problem_->getVariable(SepVars_[i][j]->getV()->getId());

            if (lf->hasVar(v)) {
              const double d2 = lf->getWeight(v);
              lf->removeVar(v, lf->getWeight(v));
              lf1->addTerm(v, d2);
            }
          }
        }

        v = problem_->newVariable(VarTran);
        newVars_++;
        lf->addTerm(v, 1);
        lf1->addTerm(v, -1);
        lfnew.push_back(lf1);
        //std::cout << std::endl << "New constraint ***: ";
        //cg[i]->write(std::cout);
        //std::cout << std::endl;
        nlfnew.push_back(cg[i]);
        newCons_++;      
        v.reset();
        lf1.reset();
      }

      lfmod.push_back(lf);
      SepVars_.clear();
      cg.clear();

      //count of separable constraints
      sepC_++;

      //Mark the original nonlinear constraint for deletion
      problem_->markDelete(c_ptr);
    }
  } 

  lf.reset();
  //Remove constraints marked as delete
  problem_->delMarkedCons();
  //std::cout << "After delMarkedCons ***: " << std::endl;
  //problem_->write(std::cout);
  //std::cout << "End of write ***: " << std::endl;

  //Add the modified linear constraints to the problem
  //std::cout << "Original nl cons to linear cons " << std::endl;
  //ConstraintPtr cp = (ConstraintPtr) new Constraint();
  for(UInt i=0; i < lfmod.size(); i++) {
    f = (FunctionPtr) new Function(lfmod[i]);
    problem_->newConstraint(f, lbMlf[i], ubMlf[i]);
    //f->write(std::cout);
    //std::cout << std::endl;
    //cp = problem_->newConstraint(f, lbMlf[i], ubMlf[i]);
    //cp->write(std::cout);
    //std::cout << std::endl;
    f.reset();
  }

  //Add new separable constraints
  //std::cout << "Separable cons " << std::endl;
  //testing
  for(UInt i=0; i < lfnew.size(); i++) {
    f = (FunctionPtr) new Function(lfnew[i], nlfnew[i] );
    problem_->newConstraint(f, -INFINITY, 0.0);
    //f->write(std::cout);
    //std::cout << std::endl;
    //cp = problem_->newConstraint(f, -INFINITY, 0.0);
    //cp->write(std::cout);
    //std::cout << std::endl;
    f.reset();
  }

  lfmod.clear();
  lfnew.clear();
  nlfnew.clear();
  ubMlf.clear();
  lbMlf.clear();
}

void TransSep::objSepCheck()
{
  bool sepStatus;
  ConstraintPtr c_ptr;
  FunctionPtr f;
  NonlinearFunctionPtr nlf;
  LinearFunctionPtr lf, lf1;
  CGraphPtr cgp;
  //CNode  *n1=0;

  UInt nvar=0, nnode=0;

  std::vector<CGraphPtr > cg; 
  CNodeQ dq; 
  ConstVariablePtr v;
  //std::vector<LinearFunctionPtr > lfnew; //Linear functions of separable cons
  //std::vector<LinearFunctionPtr > lfmod; //modified linear fun of cons
  //std::vector<CGraphPtr > nlfnew; //Linear function of new linear constraints
  //std::vector<double > ubMlf; //ub of modified linear constraints
  //std::vector<double > lbMlf; //lb of modified linear constraints
  //Checking separability of objective function
  ObjectivePtr obj;
  obj = problem_->getObjective();
  f = obj->getFunction();
  cgp = boost::dynamic_pointer_cast <CGraph> (f->getNonlinearFunction());
  // assert(cg);
  if (cgp == NULL) {
    sepStatus = false;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "Objective: "<< obj->getName() 
      << " does not have cgraph.\n"<< std::endl;
#endif

#if 0
    obj->write(std::cout);
    std::cout << " Objective does not have cgraph.\n";
#endif    
  } else {

    coeff_ = 1.0;
    bcoeff_ = false;
    //if ( obj->getLinearFunction()) {
      //lf = obj->getLinearFunction(); 
    //} else {
      //lf = (LinearFunctionPtr) new LinearFunction();
    //}

    nvar = f->getNonlinearFunction()->numVars();
    nnode = cgp->getNumNodes();
    const CNode *o = cgp->getOut();
    sepStatus = outCheck(o, nnode, nvar);

#if 0
    obj->write(std::cout);
    if (sepStatus == true) {
      std::cout << " is separable. \n";
    } else {
      std::cout << " is not separable.\n";
    }
#endif    

  }

  if (sepStatus == true ) {

    objSep_=true;

    //Generating cgraph of separable parts
    //dq contains all the dependent nodes of the cgp
    dq = cgp->dNodes(); 
    //This vector contains cgraph of separable parts.
    cg = SepCGraph(nnode, &dq);

    if ( obj->getLinearFunction()) {
      lf = obj->getLinearFunction(); 
    } else {
      lf = (LinearFunctionPtr) new LinearFunction();
    }
    //std::cout << "Separable constraints from obj " << std::endl;
    v = (VariablePtr) new Variable();
    for(UInt i=0; i < cg.size(); i++) {

      lf1 = (LinearFunctionPtr) new LinearFunction();
      if (obj->getLinearFunction()!= NULL) {
        for (UInt j=0; j< SepVars_[i].size(); j++) {
          v = problem_->getVariable(SepVars_[i][j]->getV()->getId());

          if (lf->hasVar(v)) {
            const double d2 = lf->getWeight(v);
            lf->removeVar(v, lf->getWeight(v));
            lf1->addTerm(v, d2);
          }
        }
      }

      v = problem_->newVariable(VarTran);
      newVars_++;
      lf->addTerm(v, 1);
      lf1->addTerm(v, -1);
      f = (FunctionPtr) new Function(lf1, cg[i]);
      //f->write(std::cout);    
      //std::cout << std::endl;
      //Adding separable constraints to the problem
      problem_->newConstraint(f, -INFINITY, 0.0);
      newCons_++;
      newCons_++;      
      v.reset();
      lf1.reset();
      f.reset();
    }
    SepVars_.clear();
    cg.clear();

    //Remove old objective and add a new one
    problem_->removeObjective();
    f = (FunctionPtr) new Function(lf);
    problem_->newObjective(f, obj->getConstant(), obj->getObjectiveType());

    lf.reset();
    //std::cout << "New linear obj " << std::endl;
    //ObjectivePtr obp;
    //obp = problem_->newObjective(f, obj->getConstant(), obj->getObjectiveType());
    //obp->write(std::cout);
    //std::cout << std::endl;
  }
  //std::cout << "End of function ***: " << std::endl;
  //problem_->write(std::cout);
  //std::cout << "End of write ***: " << std::endl;
} 

bool TransSep::outCheck(const CNode * o, UInt nnode, UInt nvar)
{
  CNodeQ sn;
  std::deque<int> sop;
  std::vector<CNode *> svars; 
  std::vector<CNode *> sconst;
  CNode * n1=0;

  std::stack<CNode *>  on;
  std::stack<int>  onop;

  switch(o->getOp()){
  case (OpPlus):
    on.push(o->getL());
    onop.push(1);
    on.push(o->getR());
    onop.push(1);
    return  sepCheck(nnode, &sn, &sop, &svars, &sconst, nvar, &on, &onop);
#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif
    break;
  case (OpMinus):
    on.push(o->getL());
    onop.push(1);  
    on.push(o->getR());
    onop.push(-1); 
    return  sepCheck(nnode, &sn, &sop, &svars, &sconst, nvar, &on, &onop);
#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif
    break;
  case (OpSumList):
    for (CNode **it=o->getListL(); it!=o->getListR(); ++it) {
      n1 = *it;
      on.push(n1);
      onop.push(1);
    }  
    return  sepCheck(nnode, &sn, &sop, &svars, &sconst, nvar, &on, &onop);
#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif
    break;
  case (OpUMinus):
    on.push(o->getL());
    onop.push(-1);
    return  sepCheck(nnode, &sn, &sop, &svars, &sconst, nvar, &on, &onop);
#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif

    break;
  case (OpPowK):
    if (o->getR()->getVal() == 1.0) {
      return  outCheck(o->getL(), nnode, nvar);
#if SPEW
      //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif
      break;
    } else {
      return false;
    }
  case (OpIntDiv):
    coeff_ = coeff_ *(1/ (o->getR()->getVal()));
    bcoeff_ = true;
    return  outCheck(o->getL(), nnode, nvar);
#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif
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
    // Constraint not separable.
    return false;
#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Status of Constraint : "<< c_ptr->getName()                                                                               << " separability: " << sepStatus << " .\n"<< std::endl;
#endif
    break;
  case (OpDiv):
    if (o->getR()->getOp() == OpNum || o->getR()->getOp() == OpInt) {
      bcoeff_ = true;
      coeff_ = coeff_ *(1/ (o->getR()->getVal()));
      return  outCheck(o->getL(), nnode, nvar);
    } else if (o->getL()->getOp() == OpNum || o->getL()->getOp() == OpInt) {
      bcoeff_ = true;
      coeff_ = coeff_ *(1/ (o->getL()->getVal()));
      return  outCheck(o->getR(), nnode, nvar);
    }
    else {
      return false;    
    }
    break;
  case (OpMult):
    if (o->getR()->getOp() == OpNum || o->getR()->getOp() == OpInt) {
      bcoeff_ = true;
      coeff_ =coeff_ * (o->getR()->getVal());
      return  outCheck(o->getL(), nnode, nvar);
    } else if (o->getL()->getOp() == OpNum || o->getL()->getOp() == OpInt) {
      bcoeff_ = true;
      coeff_ =coeff_*( o->getL()->getVal());
      return  outCheck(o->getR(), nnode, nvar);
    } else {
      return false;    
    }
    break;
  case (OpPow):
    assert(!"not implemented!");
    //Any message to be included?
    return false;
    break;
  case (OpNone):
    assert(!"warning: encountered node with opcode OpNode.");
    //Any message to be included?
    return false;
    break;

  default:
    assert(!"cannot evaluate!");
    //Any message to be included?
    return false;
    break;
  }
}

std::vector<CGraphPtr> TransSep::SepCGraph(UInt nnode, CNodeQ * dq)
{
  //add only that many places as required by modifying index of the nodes
  //appropriately. Select the highest index of the all nodes of
  //SepNodes_
  //std::vector<CNode *> tempN_;
  for(UInt i=0; i < nnode; i++) {
    itnum_.push_back(0);
    tempN_.push_back(NULL);
  }

  CNode * n1;
  std::vector<CGraphPtr> cg;
  UInt k=0;
  //double d1;
  //int i1;
  for(UInt i=0; i < SepNodes_.size(); ) {
    //If empty then remove
    if (SepNodes_[i].size() == 0) {
      SepNodes_.erase(SepNodes_.begin()+i);  
      SepOps_.erase(SepOps_.begin()+i);  
      SepVars_.erase(SepVars_.begin()+i);  
      SepConst_.erase(SepConst_.begin()+i);  
    } else {
      //if nonempty then generate the cgraph
      CGraphPtr t =(CGraphPtr) new CGraph();
      //add variable nodes to the cgraph directly
      for (UInt j=0; j < SepVars_[i].size(); j++) {
        //n1 = SepVars_[i][j];
        k = SepVars_[i][j]->getIndex();
        tempN_[k] = t->newNode(problem_->getVariable(SepVars_[i][j]->getV()->getId()));
        itnum_[k]= i+1; // this means n1 is in separable part k
      }
      //add constant nodes to the cgraph directly
      for (UInt j=0; j < SepConst_[i].size(); j++) {
        n1 = SepConst_[i][j];
        k = n1->getIndex();
        if (n1->getOp() == OpNum) {
          //d1 = n1->getLb();
          tempN_[k] = t->newNode(n1->getLb());
          //tempN_[k] = cg[i]->newNode(d1);
          itnum_[k]= i+1; // not required as constant node has only one parent
        }
        if (n1->getOp() == OpInt) {
          //i1 = n1->getLb();
          tempN_[k] = t->newNode(n1->getLb());
          //tempN_[k] = cg[i]->newNode(i1);
          itnum_[k]= i+1; // not required as constant node has only one parent
        }
      } 
      //Mark the dependent nodes as to which separable part they belong.
      MarkVis(i);
      //k++;
      cg.push_back(t);
      t.reset();
      i++;
    }
  }
  //itnum_.clear(); 
  SepConst_.clear();
  //No. of separable parts is equal to SepNodes_.size() or cg.size()
  //

  createCG(&cg, dq);
  finalCG(&cg);
  return cg;

}

void TransSep::createCG(std::vector<CGraphPtr > * cg, CNodeQ * dq)
{
  //generating cgraph of separable parts
  CNode **childr = 0;
  UInt k=0, c2=0, f1;
  CNode *nl=0, *nr=0, *n1 =0;
  for (UInt i=0; i < (*dq).size(); i++) {
    n1 = (*dq)[i];
    //k = itnum_[n1->getIndex()] ;
    k = itnum_[(*dq)[i]->getIndex()] ;
    if (k > 0) {
      switch(n1->getOp()) {
      case (OpAbs):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAbs, nl, 0);          
        break;
      case (OpAcos):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAcos, nl, 0);          
        break;
      case (OpAcosh):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAcosh, nl, 0);          
        break;
      case (OpAsin):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAsin, nl, 0);          
        break;
      case (OpAsinh):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAsinh, nl, 0);          
        break;
      case (OpAtan):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAtan, nl, 0);          
        break;
      case (OpAtanh):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpAtanh, nl, 0);          
        break;
      case (OpCeil):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpCeil, nl, 0);          
        break;
      case (OpCos):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpCos, nl, 0);          
        break;
      case (OpCosh):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpCosh, nl, 0);          
        break;
      case (OpCPow):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpCPow, nl, 0);          
        break;
      case (OpExp):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpExp, nl, 0);          
        break;
      case (OpFloor):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpFloor, nl, 0);          
        break;
      case (OpLog):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpLog, nl, 0);          
        break;
      case (OpLog10):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpLog10, nl, 0);          
        break;
      case (OpRound):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpRound, nl, 0);          
        break;
      case (OpSin):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpSin, nl, 0);          
        break;
      case (OpSinh):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpSinh, nl, 0);          
        break;
      case (OpSqr):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpSqr, nl, 0);          
        break;
      case (OpTan):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpTan, nl, 0);          
        break;
      case (OpTanh):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpTanh, nl, 0);          
        break;
      case (OpUMinus):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpUMinus, nl, 0);          
        break;
      case (OpSqrt):
        nl = getLchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpSqrt, nl, 0);          
        break;
      case (OpDiv):
        nl = getLchild(n1);
        nr = getRchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpDiv, nl, nr);          
        break;
       case (OpPowK):
        nl = getLchild(n1);
        nr = getRchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpPowK, nl, nr);          
        break;  
       case (OpIntDiv):
        nl = getLchild(n1);
        nr = getRchild(n1); 
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpIntDiv, nl, nr);
      case (OpPlus):
        nl = getLchild(n1);
        nr = getRchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpPlus, nl, nr);          
        break;
      case (OpMult):
        nl = getLchild(n1);
        nr = getRchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpMult, nl, nr);          
        break;
      case (OpMinus):
        nl = getLchild(n1);
        nr = getRchild(n1);
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpMinus, nl, nr);          
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
        tempN_[n1->getIndex()]=(*cg)[k-1]->newNode(Minotaur::OpSumList, childr, f1);          
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

void TransSep::finalCG(std::vector<CGraphPtr > * cg)
{
  UInt c2=0, c3=0, f1;
  CNode **childr = 0;
  CNode * n1=0, *nl =0, *n2=0, *nr=0;
  //Stage of Algo. Final step of the generation of cgraph of separable parts
  for (UInt i=0; i < SepNodes_.size(); i++) {
    //std::cout << i+1 << " cgraph before full construction " << std::endl;
    //cg[i]->write(std::cout);
    //std::cout << std::endl;
    f1 = SepNodes_[i].size();
    if (f1 == 1) {
      if (SepOps_[i][0] == 1) {
        nl = SepNodes_[i][0];
        n1 = tempN_[nl->getIndex()];
      } else {
        nl = tempN_[SepNodes_[i][0]->getIndex()];
        n1 = (*cg)[i]->newNode(Minotaur::OpUMinus, nl, 0);
      }
    }

    if (f1 == 2) {
      if (SepOps_[i][0] == SepOps_[i][1] && SepOps_[i][0] == 1) {
        c2 = SepNodes_[i][0]->getIndex();
        nl = tempN_[c2];
        c2 = SepNodes_[i][1]->getIndex();
        nr = tempN_[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpPlus, nl, nr);
      }
      if (SepOps_[i][0] == SepOps_[i][1] && SepOps_[i][0] != 1) {
        c2 = SepNodes_[i][0]->getIndex();
        n2 = tempN_[c2];
        nl = (*cg)[i]->newNode(Minotaur::OpUMinus, n2, 0);
        c2 = SepNodes_[i][1]->getIndex();
        n2 = tempN_[c2];
        nr = (*cg)[i]->newNode(Minotaur::OpUMinus, n2, 0);
        n1 = (*cg)[i]->newNode(Minotaur::OpPlus, nl, nr); 
      }
      if (SepOps_[i][0] != SepOps_[i][1] && SepOps_[i][0] == 1) {
        c2 = SepNodes_[i][0]->getIndex();
        nl = tempN_[c2];
        c2 = SepNodes_[i][1]->getIndex();
        nr = tempN_[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr);
      }
      if (SepOps_[i][0] != SepOps_[i][1] && SepOps_[i][0] != 1) {
        c2 = SepNodes_[i][0]->getIndex();
        nr= tempN_[c2];
        c2 = SepNodes_[i][1]->getIndex();
        nl = tempN_[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr);       
      }
    }
    if (f1 > 2) {
      childr = new Minotaur::CNode *[f1];
      c2=0;
      for(UInt j=0; j<SepNodes_[i].size(); j++) {
        if (SepOps_[i][j] == 1) {
          c3 = SepNodes_[i][j]->getIndex();
          childr[c2]=tempN_[c3];
          c2++;     
        } else {
          c3 = SepNodes_[i][j]->getIndex();
          n1 = tempN_[c3];
          childr[c2]=(*cg)[i]->newNode(Minotaur::OpUMinus, n1, 0);     
          c2++;
        }
      }
      n1 = (*cg)[i]->newNode(Minotaur::OpSumList, childr, f1);          
      delete []childr;
    }
   if (bcoeff_ == true) {
     nl = (*cg)[i]->newNode(coeff_);          
     nr = n1;
     n1 = (*cg)[i]->newNode(Minotaur::OpMult, nl, nr);          
   } 
    (*cg)[i]->setOut(n1);
    (*cg)[i]->finalize();

    //std::cout << i+1 << " cgraph after full construction " << std::endl;
    //(*cg)[i]->write(std::cout);
    //std::cout << std::endl;
  }
  SepNodes_.clear();
  SepOps_.clear();
  tempN_.clear();
}

void TransSep::MarkVis(UInt i)
{
  CNode *n1=0, *n2=0;
  //UInt c1, c2;
  std::stack<CNode * > ts;
  //f set to 2 if both the children of a node are already added
  for(UInt j=0; j<SepNodes_[i].size(); j++) {
    n1 = SepNodes_[i][j];
    //c=0;
    //tempN_[c]=n;
    //c1 = n->getIndex();
    //itnum_[c1] = k+1;
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
        break; //to be looked at it later.
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

//Checking separability: Stage 1 of the algorithm
bool TransSep::sepCheck(UInt  nnode, CNodeQ * snodes, 
                        std::deque<int> *sops, std::vector<CNode *> * svars,
                        std::vector<CNode *> * sconst, UInt nvar, std::stack<CNode *> *on, std::stack<int> *onop)
{
  CNode *n1 = 0, *n2 =0;
  int opc;
  UInt  j=0, mNum; // j is iteration counter, mNum for merge operation
  //Variable for merge operation
  std::vector<UInt > m;
  m.push_back(0);
  //No nodes are visited yet.
  for(UInt i=0; i < nnode; i++) {
    itnum_.push_back(0);
  }

  while ((*on).size() != 0) {
    n1=(*on).top();
    opc=(*onop).top();
    switch (n1->getOp()){ 
    case (OpNum):
    case (OpInt):
      (*snodes).push_back(n1);
      (*sops).push_back(opc);    
      (*sconst).push_back(n1);
      (*on).pop();
      (*onop).pop();
      SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();

      j++;
      m.push_back(0);
      break;
    case (OpVar):
      if (itnum_[n1->getIndex()]==0) {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        (*svars). push_back(n1);
        itnum_[n1->getIndex()]=j+1;
        (*on).pop();
        (*onop).pop();
      SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();

        j++;
        m.push_back(0);
      } else {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        //Merge operation
        mNum=MergeIt(j, itnum_[n1->getIndex()], &m);
        Merge(mNum, snodes, sops, svars, sconst);
        (*on).pop();
        (*onop).pop();
        if ((*svars).size() == nvar) {
          (*snodes).clear();
          (*sops).clear();
          (*svars).clear();
          (*sconst).clear();
          itnum_.clear(); 
          SepNodes_.clear();
          SepOps_.clear();
          SepVars_.clear();
          SepConst_.clear();
          (*on)=  std::stack<CNode *> (); // clearing stack
          (*onop) = std::stack<int > (); //clearing stack
          return false;  
        }
        SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();
        j++;
        m.push_back(0);
      }
      break;
    case (OpPlus):
      if (itnum_[n1->getIndex()]==0) {
        //itnum_[n1->getIndex()]=j+1;
        (*on).pop();
        (*onop).pop();
        (*on).push(n1->getL());
        (*onop).push(opc);
        (*on).push(n1->getR());
        (*onop).push(opc);
      } else {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        //Merge operation
        mNum=MergeIt(j, itnum_[n1->getIndex()], &m);
        Merge(mNum, snodes, sops, svars, sconst);
        (*on).pop();
        (*onop).pop();
        if ((*svars).size() == nvar) {
          (*snodes).clear();
          (*sops).clear();
          (*svars).clear();
          (*sconst).clear();
          itnum_.clear(); 
          SepNodes_.clear();
          SepOps_.clear();
          SepVars_.clear();
          SepConst_.clear();
          (*on)=  std::stack<CNode *> (); // clearing stack
          (*onop) = std::stack<int > (); //clearing stack
          return false;  
        }
        SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();
        j++;
        m.push_back(0);
      }
      break;
    case (OpMinus):
      if (itnum_[n1->getIndex()]==0) {
        //itnum_[n1->getIndex()]=j+1;
        (*on).pop();
        (*onop).pop();
        (*on).push(n1->getL());
        (*onop).push(opc);
        (*on).push(n1->getR());
        (*onop).push(opc*(-1));
      } else {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        //Merge operation
        mNum=MergeIt(j, itnum_[n1->getIndex()], &m);
        Merge(mNum, snodes, sops, svars, sconst);
        (*on).pop();
        (*onop).pop();
        if ((*svars).size() == nvar) {
          (*snodes).clear();
          (*sops).clear();
          (*svars).clear();
          (*sconst).clear();
          itnum_.clear(); 
          SepNodes_.clear();
          SepOps_.clear();
          SepVars_.clear();
          SepConst_.clear();
          (*on)=  std::stack<CNode *> (); // clearing stack
          (*onop) = std::stack<int > (); //clearing stack
          return false;  
        }
        SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();
        j++;
        m.push_back(0);
      }
      break;
    case (OpSumList):
      if (itnum_[n1->getIndex()]==0) {
        //itnum_[n1->getIndex()]=j+1;
        (*on).pop();
        (*onop).pop();
        for (CNode **it=n1->getListL(); it!=n1->getListR(); ++it) {
          n2 = *it;
          (*on).push(n2);
          (*onop).push(opc);
        } 
      } else {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        //Merge operation
        mNum=MergeIt(j, itnum_[n1->getIndex()], &m);
        Merge(mNum, snodes, sops, svars, sconst);
        (*on).pop();
        (*onop).pop();
        if ((*svars).size() == nvar) {
          (*snodes).clear();
          (*sops).clear();
          (*svars).clear();
          (*sconst).clear();
          itnum_.clear(); 
          SepNodes_.clear();
          SepOps_.clear();
          SepVars_.clear();
          SepConst_.clear();
          (*on)=  std::stack<CNode *> (); // clearing stack
          (*onop) = std::stack<int > (); //clearing stack
          return false;  
        }
        SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();
        j++;
        m.push_back(0);
      }
      break; 
    case (OpUMinus):
      if (itnum_[n1->getIndex()]==0) {
        //itnum_[n1->getIndex()]=j+1;
        (*on).pop();
        (*onop).pop();
        (*on).push(n1->getL());
        (*onop).push(opc*(-1));
      } else {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        //Merge operations
        mNum=MergeIt(j, itnum_[n1->getIndex()], &m);
        Merge(mNum, snodes, sops, svars, sconst);
        (*on).pop();
        (*onop).pop();
        if ((*svars).size() == nvar) {
          (*snodes).clear();
          (*sops).clear();
          (*svars).clear();
          (*sconst).clear();
          itnum_.clear(); 
          SepNodes_.clear();
          SepOps_.clear();
          SepVars_.clear();
          SepConst_.clear();
          (*on)=  std::stack<CNode *> (); // clearing stack
          (*onop) = std::stack<int > (); //clearing stack
          return false;  
        }
        SepNodes_.push_back((*snodes)) ;
        SepOps_.push_back((*sops)) ;
        SepVars_.push_back((*svars)) ;
        SepConst_.push_back((*sconst)) ;

        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();
        j++;
        m.push_back(0);
      }
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
    case (OpIntDiv):
    case (OpLog):
    case (OpLog10):
    case (OpRound):
    case (OpSin):
    case (OpSinh):
    case (OpSqr):
    case (OpTan):
    case (OpTanh):
    case (OpSqrt):
    case (OpPowK):
    case (OpDiv):
    case (OpMult):
      if (itnum_[n1->getIndex()]==0) {
        itnum_[n1->getIndex()]=j+1;
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        DepthFS(j, n1, snodes, sops, svars, sconst, &m);
        (*on).pop();
        (*onop).pop();
      } else {
        (*snodes).push_back(n1);
        (*sops).push_back(opc);
        //Merge
        mNum=MergeIt(j, itnum_[n1->getIndex()], &m);
        Merge(mNum, snodes, sops, svars, sconst);
        (*on).pop();
        (*onop).pop();
      }
      if ((*svars).size() == nvar) {
        (*snodes).clear();
        (*sops).clear();
        (*svars).clear();
        (*sconst).clear();
        itnum_.clear(); 
        SepNodes_.clear();
        SepOps_.clear();
        SepVars_.clear();
        SepConst_.clear();
        (*on)=  std::stack<CNode *> (); // clearing stack
        (*onop) = std::stack<int > (); //clearing stack
        return false;  
      }
      SepNodes_.push_back((*snodes)) ;
      SepOps_.push_back((*sops)) ;
      SepVars_.push_back((*svars)) ;
      SepConst_.push_back((*sconst)) ;

      (*snodes).clear();
      (*sops).clear();
      (*svars).clear();
      (*sconst).clear();
      j++;
      m.push_back(0);

      break;
      //case (OpNone):
      //break;
    case (OpNone):
      assert(!"warning: encountered a node with opcode OpNone");
      //Any message to be included?
      break;
    case (OpPow):
      assert(!"not implemented!");
      //Any message to be included?
      break;
    default:
      assert(!"cannot evaluate!");
      //Any message to be included?
      break;
    }
  }
  itnum_.clear();
  m.clear();
  return true;
}

int TransSep::MergeIt(int j, int a, std::vector<UInt > * m)
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
    return MergeIt(j, b, m); 
  }
}

void TransSep::Merge(int mNum, CNodeQ * snodes, 
                     std::deque<int > * sops, 
                     std::vector<CNode *> *svars, std::vector<CNode *> *sconst )
{
  if (SepNodes_[mNum][0]->getIndex() > (*snodes)[0]->getIndex()) {
    (*snodes).push_front(SepNodes_[mNum][0]);
    (*sops).push_front(SepOps_[mNum][0]);
    for(UInt i=1; i<SepNodes_[mNum].size(); i++) {
      (*snodes).push_back(SepNodes_[mNum][i]);
      (*sops).push_back(SepOps_[mNum][i]);
    }
  } else {
    for(UInt i=0; i<SepNodes_[mNum].size(); i++) {
      (*snodes).push_back(SepNodes_[mNum][i]);
      (*sops).push_back(SepOps_[mNum][i]);
    }
  }

  SepNodes_[mNum].clear();
  SepOps_[mNum].clear();

  for(UInt i=0; i<SepVars_[mNum].size(); i++) {
    (*svars).push_back(SepVars_[mNum][i]);
  }
  SepVars_[mNum].clear();

  for(UInt i=0; i<SepConst_[mNum].size(); i++) {
    (*sconst).push_back(SepConst_[mNum][i]);
  }
  SepConst_[mNum].clear();

}

void TransSep::DepthFS(int j, CNode *n1, CNodeQ * snodes, 
                       std::deque<int > * sops,
                       std::vector<CNode *> *svars, std::vector<CNode *> *sconst, std::vector<UInt > *m )
{
  std::stack<CNode *> tempNodes;
  CNode *n=0;
  CNode *n2=0;
  int mNum;
  //Initialization of temp nodes in T
  TempPop(n1, &tempNodes);
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
        //case (OpIntDiv):
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
        //Check if opNone is to be pushed here
        tempNodes.pop();
        (*sconst).push_back(n);
        break;
      case (OpVar):
        tempNodes.pop();
        (*svars).push_back(n);
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
      //mNum=0;
      if (itnum_[n->getIndex()] == j+1) {
        tempNodes.pop();      
      } else {
        //Merge
        mNum=MergeIt(j, itnum_[n->getIndex()], m);
        if (mNum > -1) {
          Merge(mNum, snodes, sops, svars, sconst);
        }
        tempNodes.pop();      
      }
    }
  }
}

void TransSep::TempPop(CNode *n1, std::stack<CNode *> * tempNodes)
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
  case (OpNone): //To be handled
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

void TransSep::disProb()
{
  //wrap up.
  env_->getLogger()->msgStream(LogInfo) << me_ << "Finished separability detection"
    << std::endl;
  problem_->calculateSize(true);
  if (true == env_->getOptions()->findBool("display_separable_size")->
      getValue()) {
    problem_->writeSize(logger_->msgStream(LogNone));
  }
  if (true ==
      env_->getOptions()->findBool("display_separable_problem")->
      getValue()) {
    problem_->write(logger_->msgStream(LogNone));
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
