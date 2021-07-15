// MINOTAUR -- It's only 1/2 bull
// 
// (C)opyright 2008 - 2021 The MINOTAUR Team.
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
#include "QuadraticFunction.h"
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
:env_(EnvPtr()), problem_(ProblemPtr()), newVars_(0), sepNodes_(0),
  sepOps_(0), sepVars_(0), sepConst_(0), sepCons_(0), intTol_(1e-6)
{
}


TransSep::TransSep(EnvPtr env, ProblemPtr problem)
:env_(env), problem_(problem), newVars_(0), sepNodes_(0),
  sepOps_(0), sepVars_(0), sepConst_(0), sepCons_(0)
{
  intTol_ = env_->getOptions()->findDouble("int_tol")->getValue();
  logger_   = (LoggerPtr) new Logger((LogLevel) env_->getOptions()->
                                     findInt("separability_log_level")->
                                     getValue());
}


TransSep::~TransSep()
{
  env_ = 0;
  logger_ = 0;
  problem_ = 0;
}

//UInt TransSep::eqnDirection(ConstraintPtr cptr) 
//{
  //double lb, ub;
  //lb = cptr->getLb();
  //ub = cptr->getUb();
  //if (ub == INFINITY && lb != -INFINITY) { 
    //return 0;
  //} else if (ub != INFINITY && lb == -INFINITY) {
    //return 1;
  //}  else if (ub != INFINITY && lb != -INFINITY) {
    //return 2;
  //} else {
//#if SPEW
    //logger_->msgStream(LogDebug) << me_ << "Redundant constraint" << std::endl;
//#endif
    //problem_->markDelete(c_ptr);
    //return 3;
  //}
//}

void TransSep::checkCons()
{
//testing

  //std::vector<std::string > cname1;
  //UInt count1=0, nlCons = 0;
  //bool objSep = 0;
//testing end
  CNodeQ dq; 
  FunctionPtr f;
  CGraphPtr cgp;
  ConstVariablePtr v;

  std::vector<UInt > nsp, eqs; // No. of separable parts in separable constraint; eqs=0 if cons is lb <= f(x), 1 if f(x) <= ub
  std::vector<std::string > sepConName;
  std::vector<std::string > modConName;
  
  std::stack<int > candOp;
  LinearFunctionPtr lf, lf1;
  //NonlinearFunctionPtr nlf;
  ConstraintPtr c_ptr, cOriCopy;
  std::stack<CNode *> candNodes;
  double coeff, ub, consVal;
  std::vector<double > ubMlf; 
  VariableGroupConstIterator vit;
  std::vector<double > constCoeff;
  std::vector<CGraphPtr > cg, nlfnew;     // Nonlinear functions of separable cons
  std::vector<double > hashVal, hashVal1;     // hash val for new Nonlinear functions of separable cons
  std::vector<LinearFunctionPtr > lfnew, lfmod;  // Linear functions of separable cons and modified linear fun of cons
  bool sepStatus = false, status;
  UInt sepIntensity = env_->getOptions()->findInt("separability_intensity_level")
    ->getValue();
  UInt pVars = problem_->getNumVars(), n1, n2;
  
  double *x = new double[pVars]; //MS: Move from here to an appropriate place?
  for (UInt i = 0; i < pVars; ++i) {
    x[i] = 1 + (i+1)*.5;  
  }
  
  for (ConstraintConstIterator c_iter = problem_->consBegin();
      c_iter != problem_->consEnd(); ++c_iter) {
    
    c_ptr = *c_iter;
    f = c_ptr->getFunction();
    cgp = dynamic_cast <CGraphPtr> (f->getNonlinearFunction());
    
    if (cgp == NULL) {
      continue;
    } else {
      //++nlCons;
      //c_ptr->write(std::cout);
      //cgp->removeNodes();
      //c_ptr->write(std::cout);
      consVal = 0.0;
      ub = c_ptr->getUb();
      
      if (ub == INFINITY) { 
        continue;
      }

      if (c_ptr->getLinearFunction()) {
        lf = c_ptr->getLinearFunction();
      } 

      status = cgp->ifLinear(lf, pVars, &consVal);
      
      if (status) {
        if (lf->getNumTerms() == 0) {
          problem_->markDelete(c_ptr);
          continue;
        }
        //MS: ensure this!
        //if (c_ptr->getFunctionType() == Quadratic || 
            //c_ptr->getQuadraticFunction()) {
          //c_ptr->changeQf_(QuadraticFunctionPtr());
        //} 
        
        c_ptr->changeLf_(lf);
        c_ptr->setUb_(ub-consVal);
        c_ptr->changeNlf_(NonlinearFunctionPtr());
        //c_ptr->write(std::cout);
        continue;      
      }

      if (c_ptr->getLinearFunction()) {
        lf = c_ptr->getLinearFunction()->clone();
      } else {
        lf = (LinearFunctionPtr) new LinearFunction();
      }
      
      coeff = 1;
      n1 = cgp->getNumNodes();
      const CNode *o = cgp->getOut();
      n2 = f->getNonlinearFunction()->numVars();
      sepStatus = rootChildren(o,&candNodes,&candOp,&coeff,n1,n2,&ub,lf);

      if (coeff == 0) {
        problem_->markDelete(c_ptr);
        continue;
      }

      if (sepStatus) {
        // store the original constraint
        cOriCopy = (ConstraintPtr) new Constraint(c_ptr->getId(),
                                                  c_ptr->getIndex(),
                                                  c_ptr->getFunction(),
                                                  c_ptr->getLb(), c_ptr->getUb(),
                                                  "Ori_cons_before_sep_rformulation");
        sepCons_.push_back(cOriCopy); //MS: clear sepCons later!
  
        /// Generating cgraph of separable parts
        dq = cgp->dNodes(); 
        
        /// vector cg contains cgraph of separable parts.
        cg = sepPartsCGraph(&dq,n1,&constCoeff,&coeff,&hashVal1,x);
        
        if (sepIntensity == 1) {        
          for (UInt i = 0; i < cg.size();) {
            status = false;
            for (UInt j = 0; j < hashVal.size(); ++j) {
              if (hashVal[j] == hashVal1[i]) {
                status = sepPartsSearch(sepConName.size(),&nlfnew,&lfnew,cg[i],
                                        constCoeff[i],lf,&nsp,coeff);             

                if (status) {
                  //++count1;
                  cg.erase(cg.begin()+i);
                  hashVal1.erase(hashVal1.begin()+i);
                  constCoeff.erase(constCoeff.begin()+i);
                  break;
                }
              }
            }
            if (!status) {
              ++i;            
            }
          }
        } 

        if (!(cg.size())) {
          ubMlf.push_back(ub);
          lfmod.push_back(lf);
          hashVal1.clear();
          sepOps_.clear();
          sepVars_.clear();
          sepConst_.clear();
          sepNodes_.clear();
          constCoeff.clear();
          candOp = std::stack<int > ();
          candNodes = std::stack<CNode *> ();
          modConName.push_back(c_ptr->getName());
          //cname1.push_back(c_ptr->getName()); //MS: need?
          problem_->markDelete(c_ptr);
          continue;
        }
        for (UInt i = 0; i < cg.size(); ++i) {
          v = problem_->newVariable(VarSep);
          v->setFunType_(Nonlinear);
          lf1 = (LinearFunctionPtr) new LinearFunction();
          lf->addTerm(v,coeff*constCoeff[i]);
          lf1->addTerm(v,-1);
          lfnew.push_back(lf1);
          hashVal.push_back(hashVal1[i]);
          nlfnew.push_back(cg[i]);
        }
        ubMlf.push_back(ub);
        //lbMlf.push_back(lb);
        lfmod.push_back(lf);
        nsp.push_back(cg.size());
        newVars_ = newVars_ + cg.size();
        sepConName.push_back(c_ptr->getName());
        modConName.push_back(c_ptr->getName());
        cg.clear();
        hashVal1.clear();
        problem_->markDelete(c_ptr);
        // lf1 reset??
      }
      sepOps_.clear();
      sepVars_.clear();
      sepConst_.clear();
      sepNodes_.clear();
      constCoeff.clear();
      candOp = std::stack<int > ();
      candNodes = std::stack<CNode *> ();
    }
  } 
  delete [] x;

//Add new constraints corresponding to separable parts to the problem
  n1 = 0;
  problem_->delMarkedCons();
  if (sepIntensity == 0) {
    for (UInt j = 0; j < sepConName.size() ; ++j) {
      n2 = n1 + nsp[j];
      for (UInt k = n1; k < n2; ++k) {
        f = (FunctionPtr) new Function(lfnew[k], nlfnew[k]);
        problem_->newConstraint(f, -INFINITY, 0.0);
      }
      n1 = n2;
      f = (FunctionPtr) new Function(lfmod[j]);
      problem_->newConstraint(f, -INFINITY, ubMlf[j],sepConName[j]);

      //testing 
      //if (sepConName[j] == "obj_Reform") {
        //objSep = 1;      
      //}
      //testing

      //f.reset();
    }
  } else {
     for (UInt j = 0; j < sepConName.size(); ++j) {
      n2 = n1 + nsp[j];
      for (UInt k = n1; k < n2; ++k) {
        f = (FunctionPtr) new Function(lfnew[k], nlfnew[k]);
        problem_->newConstraint(f, -INFINITY, 0.0);
      }
      n1 = n2;
      //if (sepConName[j] == "obj_Reform") {
        //objSep = 1;      
      //}
    }

    for (UInt i = 0; i < lfmod.size(); ++i) {
      f = (FunctionPtr) new Function(lfmod[i]);
      problem_->newConstraint(f, -INFINITY, ubMlf[i], modConName[i]);
      //if (modConName[i] == "obj_Reform") {
        //objSep = 1;      
      //}
    }
  }
    
  //std::cout <<"No. of unique separable parts: " << newVars_ << std::endl;
  //std::cout <<"No. of repeated separable parts: " << count1 << std::endl;
  //if (cname1.size()) {
    //std::cout <<"Name of separable constraints under only sep parts: ";
    //for (UInt i=0; i<cname1.size()-1; ++i) {
      //std::cout << cname1[i] <<", ";
    //}
    //std::cout << cname1[cname1.size()-1] <<"." << std::endl;
  //}

//testing

  //logger_->msgStream(LogInfo) << me_ <<"No. of separable constraints: "
    //<< modConName.size() << std::endl;
    
  ////std::cout <<"No. of tot cons, sep cons, unique part, repeated part: " << problem_->getNumCons() 
  //std::cout <<"No. of tot nlcons, obj sep, sep cons, tot uni sep parts, repeated part: " << nlCons  
    //<< " " << objSep << " " << modConName.size() << " " << newVars_ << " " << count1 << std::endl;
  //if (sepConName.size()) {
    //std::cout << "Name of separable constraints: ";
    //for (UInt i=0; i<sepConName.size()-1; ++i) {
      //std::cout << sepConName[i] <<", ";
    //}
    //std::cout << sepConName[sepConName.size()-1] << "." << std::endl;
    //std::cout <<"All sep Constraints: ";
    //for (UInt i=0; i<modConName.size()-1; ++i) {
      //std::cout << modConName[i] <<", ";
    //}
    //std::cout << modConName[modConName.size()-1] << "." << std::endl;
  //}
//testing
  //problem_->write(std::cout);
}

            
bool TransSep::sepPartsSearch(UInt sp, std::vector<CGraphPtr > *nlfnew,
                              std::vector<LinearFunctionPtr > *lfnew,
                              CGraphPtr cg, double constCoeff,
                              LinearFunctionPtr lf, std::vector<UInt > *nsp,
                              //std::vector<UInt > *eqs, UInt edir1, double coeff)
                              double coeff)
{
  UInt n1, n2;
  bool status;
  n2 = 0;
  for (UInt k = 0; k < sp; ++k) {
    status = false;
    n1 = n2;
    n2 = n1 + (*nsp)[k];
    for (UInt j = n1; j < n2; ++j) {
      //if (edir1==(*eqs)[k]) {
        status = ifRepeated((*nlfnew)[j],(*lfnew)[j],cg,
                               constCoeff,coeff,lf);
        if (status) {
          return true;
        } 
      //}
    }
  }
  return false;
}

bool TransSep::ifRepeated(CGraphPtr cgp,
                          LinearFunctionPtr lfnew,
                        CGraphPtr cg,
                       double constCoeff, double coeff, LinearFunctionPtr lf)
{
  bool status;
  ConstVariablePtr v;
  status = cg->isIdenticalTo(cgp);
  if (status) {
    for(VariableGroupConstIterator vit = lfnew->termsBegin(); vit != lfnew->termsEnd(); ++vit) {
      v   = vit->first;
      if (lf->hasVar(v)) {
        lf->incTerm(v,-1*coeff*constCoeff*(vit->second));
      } else {
        lf->addTerm(v,-1*coeff*constCoeff*(vit->second));
      }
    }
    return true;
  }
 return false;
} 
   

void TransSep::createCG(std::vector<CGraphPtr > * cg, CNodeQ * dq, int *itnum, CNode **tempN)
{
  //Generate cgraph of separable parts
  CNode **childr;
  CNode *n1=0, *nl=0, *nr = 0, *nn = 0;
  UInt k = 0, c2 = 0, f1;

  for (UInt i=0; i<(*dq).size(); ++i) {
    n1 = (*dq)[i];
    k  = itnum[n1->getIndex()];
    if (k) {
      switch (n1->getOp()) {
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
        nl                    = tempN[n1->getL()->getIndex()];
        tempN[n1->getIndex()] = (*cg)[k-1]->newNode(n1->getOp(), nl, nn);          
        break;
      case (OpDiv):
      case (OpPowK):
      case (OpPlus):
      case (OpMult):
      case (OpMinus):
      case (OpIntDiv):
        nl                    = tempN[n1->getL()->getIndex()];
        nr                    = tempN[n1->getR()->getIndex()];
        tempN[n1->getIndex()] = (*cg)[k-1]->newNode(n1->getOp(), nl, nr);          
        break;
      case (OpSumList):
        c2     = 0;
        f1     = n1->numChild();
        childr = new CNode *[f1];
        for (CNode **it = n1->getListL(); it != n1->getListR(); ++it, ++c2) {
          nl         = *it;
          childr[c2] = tempN[nl->getIndex()];
        }
        tempN[n1->getIndex()] = (*cg)[k-1]->newNode(Minotaur::OpSumList,childr,f1);
        delete [] childr;
        break;
      case (OpNone):
      case (OpNum):
      case (OpInt):
      case (OpVar):
      case (OpPow):
      default:
        break;
      }
    }
  }
}


void TransSep::depthFS(int j, CNode *n1, std::vector<UInt > *m, int *itnum,
                       CNodeQ *sNodes, std::deque<int > *sOps,
                       std::vector<CNode *> *sVars,
                       std::vector<CNode *> *sConsts)
{
  int mNum;
  CNode *n = 0;

  std::stack<CNode *> tempNodes;
  tempPopulate(n1, &tempNodes);

  while (tempNodes.size() != 0) {
    n = tempNodes.top();
    tempNodes.pop();
    if (itnum[n->getIndex()] == 0) {
      itnum[n->getIndex()] = j;
      switch (n->getOp()) {
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
        tempNodes.push(n->getL());
        break;
      case (OpDiv):
      case (OpPowK):
      case (OpPlus):
      case (OpMult):
      case (OpMinus):
      case (OpIntDiv):
        tempNodes.push(n->getR());
        tempNodes.push(n->getL());
        break;
      case (OpNum):
      case (OpInt):
        (*sConsts).push_back(n);
        break;
      case (OpVar):
        (*sVars).push_back(n);
        break;
      case (OpSumList):
        for (CNode **it=n->getListL(); it!=n->getListR(); ++it) {
          tempNodes.push(*it);
        }
        break;
      case (OpPow):
      case (OpNone):
      default:
        break;
      }
    } else {        
      if (itnum[n->getIndex()] != j) {
        mNum = mergeItrNum(j, itnum[n->getIndex()], m);
        if (mNum > -1) { 
          mergeItrInfo(mNum,sNodes,sOps,sVars,sConsts);
        }
      }
    }
  }
}


bool TransSep::explore(UInt *j, CNode *n1, std::vector<UInt > *m, int opc,
                       int *itnum, UInt nvar, CNodeQ *sNodes,
                       std::deque<int > *sOps, std::vector<CNode *> *sVars,
                       std::vector<CNode *> *sConsts)
{
  (*sNodes).push_back(n1);
  (*sOps).push_back(opc);
  if (itnum[n1->getIndex()] == 0) {
    itnum[n1->getIndex()]=*j+1;
    depthFS(*j+1,n1,m,itnum,sNodes,sOps,sVars,sConsts);

    if ((*sVars).size() == nvar) {
      return false;  
    }
  // Populate with separable part information
    sepOps_.push_back((*sOps)) ;
    sepVars_.push_back((*sVars)) ;
    sepConst_.push_back((*sConsts)) ;
    sepNodes_.push_back((*sNodes)) ;
    (*sOps).clear();
    (*sVars).clear();
    (*sConsts).clear();
    (*sNodes).clear();
  // Look for next separable part, if present.
    (*j)++;
    (*m).push_back(0);
  } else {
    UInt mNum;
    mNum = mergeItrNum(*j+1, itnum[n1->getIndex()], m);
    mergeItrInfo(mNum,sNodes,sOps,sVars,sConsts);
  }
  return true;
}


bool TransSep::coeffValue(CNode * n2, double *c,bool *n, CNode **n1,
                          std::vector<UInt > *idx)
{
  CNode *nl, *nr;
  switch (n2->getOp()) {
  case (OpDiv):
    nr = n2->getR();
    if (nr->getOp()!=Minotaur::OpNum && nr->getOp()!=Minotaur::OpInt) {
      return false;
    }
    // fall through
  case (OpIntDiv):
    *c = (*c)*(nr->getVal());
    *n = 1;        
    *n1 = n2;
    n2 = n2->getL();
    (*idx).push_back(nr->getIndex());
    coeffValue(n2,c,n,n1,idx);
    return true;
    break;
  case (OpMult):
    nl = n2->getL();
    nr = n2->getR();
    if (nl->getOp()==Minotaur::OpNum || nl->getOp()==Minotaur::OpInt) {        
      *c = (*c)*(nl->getVal());
      *n = 0;        
      *n1 =n2;
      (*idx).push_back(nl->getIndex());
      n2 = n2->getR();
      coeffValue(n2,c,n,n1,idx);
      return true;
    } else if (nr->getOp()==Minotaur::OpNum || nr->getOp()==Minotaur::OpInt) {
      *c = (*c)*(nr->getVal());
      *n = 1;        
      *n1 = n2;
      n2 = n2->getL();
      (*idx).push_back(nr->getIndex());
      coeffValue(n2,c,n,n1,idx);
      return true;
    } else {
      return false;    
    }
    break;
  default:
    return false;
    break;
  }
  return false;
}

void TransSep::commonConsCheck(std::vector<double > *constCoeff)
{
  bool status, n;
  double c;
  CNode *n1 = new CNode();
  std::vector<bool > np;
  std::vector<double > cv;
  std::vector<CNode *> nn;
  std::vector<UInt > idx;
  for (UInt i = 0; i < sepNodes_.size(); ++i) {
    (*constCoeff).push_back(1);
    c = 1;
    status = coeffValue(sepNodes_[i][0],&c,&n,&n1,&idx);
    if (!status) {
      np.clear();
      cv.clear();
      nn.clear();
      idx.clear();
      continue;    
    } else {
      cv.push_back(c);
      np.push_back(n);
      nn.push_back(n1);
    }
    for (UInt j=1;j<sepNodes_[i].size();++j) {
      c = 1;
      status = coeffValue(sepNodes_[i][j],&c,&n,&n1,&idx);
      cv.push_back(c);
      if (!status || cv[j]!=cv[j-1]) {
        status = false;
        break;      
      }
      np.push_back(n);
      nn.push_back(n1);
    }
    if (status) {
      (*constCoeff)[i] = cv[0];
      for (UInt j=0; j<cv.size(); ++j) {
        if (np[j]==1) {
          sepNodes_[i][j] = nn[j]->getL();
        } else {
          sepNodes_[i][j] = nn[j]->getR();
        }
      }

      for (UInt j=0; j<idx.size(); ++j) {
        for (UInt k=0;j<sepConst_[i].size();++k) {
          if (idx[j]==sepConst_[i][k]->getIndex()) {
            sepConst_[i].erase(sepConst_[i].begin()+k);  
            break;
          }        
        }      
      }
    }
    np.clear();
    cv.clear();
    nn.clear();
    idx.clear();
  }
}


void TransSep::finalCG(std::vector<CGraphPtr > * cg, CNode **tempN,
                       std::vector<double > *constCoeff, double *coeff,
                       std::vector<double > *hashVal1,double *x)
{
  double val;
  int error = 0;

  CNode **childr  = 0;
  UInt c2 = 0, c3 = 0, f1;
  CNode * n1 = 0, *nl = 0, *n2 = 0, *nr = 0;
  for (UInt i = 0; i < sepNodes_.size(); i++) {
    f1 = sepNodes_[i].size();
    if (f1==1) {
      c2 = sepNodes_[i][0]->getIndex();
      if (sepOps_[i][0]==-1) {
        n2 = tempN[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpUMinus, n2, 0);
      } else {
        n1 = tempN[c2];
      }
    } else if (f1==2) {
      if (sepOps_[i][0]==sepOps_[i][1] && sepOps_[i][0]==1) {
        c2 = sepNodes_[i][0]->getIndex();
        nl = tempN[c2];
        c2 = sepNodes_[i][1]->getIndex();
        nr = tempN[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpPlus, nl, nr);
      }
      if (sepOps_[i][0]==sepOps_[i][1] && sepOps_[i][0]==-1) {
        c2 = sepNodes_[i][0]->getIndex();
        n2 = tempN[c2];
        nl = (*cg)[i]->newNode(Minotaur::OpUMinus, n2, 0);
        nr = tempN[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr); 
      }
      if (sepOps_[i][0]!=sepOps_[i][1] && sepOps_[i][0]==1) {
        c2 = sepNodes_[i][0]->getIndex();
        nl = tempN[c2];
        c2 = sepNodes_[i][1]->getIndex();
        nr = tempN[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr);
      }
      if (sepOps_[i][0]!=sepOps_[i][1] && sepOps_[i][0]==-1) {
        c2 = sepNodes_[i][0]->getIndex();
        nr = tempN[c2];
        c2 = sepNodes_[i][1]->getIndex();
        nl = tempN[c2];
        n1 = (*cg)[i]->newNode(Minotaur::OpMinus, nl, nr);       
      }
    } else {  
      childr = new CNode *[f1];
      c2     = 0;
      for (UInt j=0; j<sepNodes_[i].size(); j++) {
        if (sepOps_[i][j]==1) {
          c3 = sepNodes_[i][j]->getIndex();
          childr[c2]=tempN[c3];
          c2++;     
        } else {
          c3 = sepNodes_[i][j]->getIndex();
          n1 = tempN[c3];
          childr[c2]=(*cg)[i]->newNode(Minotaur::OpUMinus, n1, 0);     
          c2++;
        }
      }
      n1 = (*cg)[i]->newNode(Minotaur::OpSumList, childr, f1);          
      delete []childr;
    }
    if ((*coeff)*((*constCoeff)[i])<=-1.0) {
      nl = (*cg)[i]->newNode(-1.0);          
      nr = n1;
      n1 = (*cg)[i]->newNode(Minotaur::OpMult, nl, nr);          //MS:No need to write Minotaur::
    }
    (*constCoeff)[i]=fabs((*constCoeff)[i]);
    (*cg)[i]->setOut(n1);
    (*cg)[i]->finalize();
    val = (*cg)[i]->eval(x,&error);
    if (error>0)  {
      val = (*cg)[i]->getNumNodes();    
      error = 0;
    }
    (*hashVal1).push_back(val);
  }
  *coeff = fabs(*coeff); 
  return;
}


//void TransSep::sepDetection(std::vector<ConstraintPtr> *sepCons) 
void TransSep::sepDetection() // MS: why two functions??
{
  //std::vector<ConstraintPtr> *sepCons;//MS:?

  // Linearize the objective if nonlinear by adding auxiliary variable
  linearizeObj();

  // Check constraint function separability
  //checkCons(sepCons);
  checkCons();

  //MS: does for objective

  // Reset initial points of the new variables added
  if (newVars_ > 0) {
    //MS: reset initial point directly as in Problem.h
    //problem_->resetInitialPoint(newVars_);
  } 

  // Write reformulated problem
  writeProb();
}


void TransSep::markVis(UInt i, int *itnum)
{
  CNode *n1 = 0, *n2 = 0;
  std::stack<CNode * > ts;

  for (UInt j=0; j<sepNodes_[i].size(); ++j) {
    //std::cout <<"Part " << i << " : " ;
    ts.push(sepNodes_[i][j]);
    while (ts.size()!=0) {
      n1                    = ts.top();
      //std::cout <<n1->getIndex() << ", " ;
      itnum[n1->getIndex()] = i+1;
      ts.pop();
      switch (n1->getOp()) {
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
        if (itnum[n1->getL()->getIndex()] == 0) { 
          ts.push(n1->getL());
        }
        break;
      case (OpDiv):
      case (OpPlus):
      case (OpMult):
      case (OpMinus):
        if (itnum[n1->getL()->getIndex()] == 0) { 
          ts.push(n1->getL());
        }
        if (itnum[n1->getR()->getIndex()] == 0) { 
          ts.push(n1->getR());
        }
        break;
      case (OpPowK):
      case (OpIntDiv):
        if (itnum[n1->getL()->getIndex()] == 0) { 
          ts.push(n1->getL());
        }
        ts.push(n1->getR());
        break;
      case (OpSumList):
        for (CNode **it = n1->getListL(); it != n1->getListR(); ++it) {
          n2 = *it;
          if (itnum[n2->getIndex()] == 0) { 
            ts.push(n2);
          }
        }           
        break;
      case (OpNone):
      case (OpNum):
      case (OpInt):
      case (OpVar):
      case (OpPow):
      default:
        break;
      }
    }
  }
  return;
}


void TransSep::mergeItrInfo(int mNum, CNodeQ *sNodes, std::deque<int > *sOps,
                            std::vector<CNode *> *sVars,
                            std::vector<CNode *> *sConsts)
{
  if (sepNodes_[mNum][0]->getIndex()>(*sNodes)[0]->getIndex()) {
    (*sOps).push_front(sepOps_[mNum][0]);
    (*sNodes).push_front(sepNodes_[mNum][0]);
  } else {
    (*sOps).push_back(sepOps_[mNum][0]);
    (*sNodes).push_back(sepNodes_[mNum][0]);
  }

  for (UInt i=1; i<sepNodes_[mNum].size(); ++i) {
    (*sOps).push_back(sepOps_[mNum][i]);
    (*sNodes).push_back(sepNodes_[mNum][i]);
  }
  sepOps_[mNum].clear();
  sepNodes_[mNum].clear();

  for( UInt i=0; i<sepVars_[mNum].size(); ++i) {
    (*sVars).push_back(sepVars_[mNum][i]);
  }
  sepVars_[mNum].clear();

  for (UInt i=0; i<sepConst_[mNum].size(); ++i) {
    (*sConsts).push_back(sepConst_[mNum][i]);
  }
  sepConst_[mNum].clear();
}


int TransSep::mergeItrNum(int j, int a, std::vector<UInt > * m)
{
  int b;
  if ((*m)[a-1] == 0) {
    (*m)[a-1] = j;
    return (a-1);
  } else if ((*m)[a-1] == (UInt) j) {
    return -1;
  } else {
    b = (*m)[a-1];
    (*m)[a-1] = j;
    return mergeItrNum(j, b, m); 
  }
}


void TransSep::linearizeObj()
{
  ObjectivePtr obj = problem_->getObjective();
  if (!obj) {
    assert(!"No objective function in the problem!");
  }
  const NonlinearFunctionPtr nlf = problem_->removeNonlinFromObj();

  if (nlf) {
    FunctionPtr fold, fnew;
    std::string name = "eta";
    LinearFunctionPtr lfold = obj->getLinearFunction();
    QuadraticFunctionPtr qf = problem_->removeQuadFromObj();
    LinearFunctionPtr lfnew = (LinearFunctionPtr) new LinearFunction();
    VariablePtr vPtr = problem_->newVariable(-INFINITY, INFINITY, Continuous, name, VarSep); 

    if (!lfold) {
      lfold = (LinearFunctionPtr) new LinearFunction();
    } else {
      lfold = obj->getLinearFunction()->clone();
    }
    
    lfold->addTerm(vPtr, -1.0);
    
    if (nlf) {
      fold = (FunctionPtr) new Function(lfold, nlf);
      if (qf) {
        fold = (FunctionPtr) new Function(lfold, qf, nlf);
      }
    }
    
    name = "obj_Reform";
    problem_->newConstraint(fold, -INFINITY, 0.0, name);
    problem_->removeObjective();
    lfnew->addTerm(vPtr, 1.0);
    fnew = (FunctionPtr) new Function(lfnew);
    problem_->newObjective(fnew, obj->getConstant(), obj->getObjectiveType());
  }
}


bool TransSep::rootChildren(const CNode * o, std::stack<CNode *> *candNodes,
                            std::stack<int > * candOp, double *coeff,
                            UInt nnode, UInt nvar, double *ub,
                            LinearFunctionPtr lf)
{
  if (fabs(*coeff) < intTol_) {
    coeff = 0;
    return false;  
  }

  switch (o->getOp()) {
  case (OpPlus):
    (*candNodes).push(o->getL());
    (*candOp).push(1);
    (*candNodes).push(o->getR());
    (*candOp).push(1);
    break;
  case (OpMinus):
    (*candNodes).push(o->getL());
    (*candOp).push(1);  
    (*candNodes).push(o->getR());
    (*candOp).push(-1); 
    break;
  case (OpSumList):
    for (CNode **it = o->getListL(); it != o->getListR(); ++it) {
      (*candNodes).push(*it);
      (*candOp).push(1);
    }  
    break;
  case (OpUMinus):
    *coeff = (*coeff)*(-1);
    return rootChildren(o->getL(),candNodes,candOp,coeff,nnode,nvar,ub,lf);
    break;
  case (OpPowK):
    if (o->getR()->getVal() == 1.0) {
      return rootChildren(o->getL(),candNodes,candOp,coeff,nnode,nvar,ub,lf);
      break;
    } else {
      return false;
    }
  case (OpIntDiv):
    *coeff = *coeff *(1/ (o->getR()->getVal()));
    return rootChildren(o->getL(),candNodes,candOp,coeff,nnode,nvar,ub,lf);
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
      *coeff = *coeff *(1/ (o->getR()->getVal()));
      return rootChildren(o->getL(),candNodes,candOp,coeff,nnode,nvar,ub,lf);
    } else {
      return false;    
    }
    break;
  case (OpMult):
    if (o->getR()->getOp() == OpNum || o->getR()->getOp() == OpInt) {
      *coeff = *coeff * (o->getR()->getVal());
      return rootChildren(o->getL(),candNodes,candOp,coeff,nnode,nvar,ub,lf);
    } else if (o->getL()->getOp() == OpNum || o->getL()->getOp() == OpInt) {
      *coeff = *coeff*( o->getL()->getVal());
      return rootChildren(o->getR(),candNodes,candOp,coeff,nnode,nvar,ub,lf);
    } else {
      return false;    
    }
    break;
  case (OpPow):
  case (OpNone):
  default:
#if SPEW
    logger_->msgStream(LogError) << me_ << "Opcode unknown." << std::endl;
#endif
    return false;
    break;
  }

  if (fabs(*coeff) < intTol_) {
    coeff = 0;
    return false;  
  }

  return sepCheck(*coeff, candNodes, candOp, nnode, nvar, ub, lf);
}


std::vector<CGraphPtr> TransSep::sepPartsCGraph(CNodeQ * dq, UInt nnode,
                                                std::vector<double > *constCoeff,
                                                double *coeff,
                                                std::vector<double > *hashVal1,
                                                double *x)
{
  UInt k;
  CNode *n1;
  std::vector<CGraphPtr> cg;

  if (env_->getOptions()->findInt("separability_intensity_level")->getValue() > 0) {
      commonConsCheck(constCoeff);
  } else {
    for (UInt i = 0; i < sepNodes_.size(); ++i) {
      (*constCoeff).push_back(1);
    }
  }
  int *itnum   = new int[nnode]();
  CNode **tempN = new CNode * [nnode]();
 
  for (UInt i = 0; i < sepNodes_.size(); ++i) {
    CGraphPtr t = (CGraphPtr) new CGraph();
    //std::cout << " Vars of sep part " << i << " :\n "; 
    for (UInt j = 0; j < sepVars_[i].size(); ++j) {
      //std:: cout << " Index: " << sepVars_[i][j]->getIndex() << " Name " << sepVars_[i][j]->getV()->getName() <<"\n";
      k = sepVars_[i][j]->getIndex();
      itnum[k] = i+1;                       //n1 is in separable part i+1
      tempN[k] = t->newNode(problem_->getVariable(sepVars_[i][j]->getV()->getIndex()));
      //std::cout << (problem_->getVariable(sepVars_[i][j]->getV()->getId()))->getName() << std::endl;
    }
    //Add constant nodes to the cgraph directly
    for (UInt j = 0; j < sepConst_[i].size(); ++j) {
      n1 = sepConst_[i][j];
      k = n1->getIndex();
      itnum[k] = i+1; 
      tempN[k] = t->newNode(n1->getVal());
    } 
    //Mark the dependent nodes as to which separable part they belong.
    markVis(i, itnum);
    cg.push_back(t);
    //t.reset();
  }
  createCG(&cg, dq, itnum, tempN);
  finalCG(&cg, tempN,constCoeff,coeff,hashVal1,x);
  delete [] itnum;
  delete [] tempN;
  return cg;
}


bool TransSep::sepCheck(double coeff, std::stack<CNode *> *candNodes,
                        std::stack<int > * candOp, UInt nnode, UInt nvar,
                        double *ub, LinearFunctionPtr lf)
{
  int opc;
  UInt itr = 0;               // iteration counter
  CNodeQ sNodes;
  std::deque<int > sOps;
  CNode *n1 = 0, *n2 = 0;
  std::vector<CNode *> sVars;
  std::vector<CNode *> sConsts;
  
  std::vector<UInt > m;         // m is used in merge operation
  int *itnum = new int[nnode]();
  
  m.push_back(0);               // Corresonding to iteration 0

  while ((*candNodes).size() != 0) {
    opc = (*candOp).top();
    n1  = (*candNodes).top();
    
    (*candOp).pop();
    (*candNodes).pop();
    
    switch (n1->getOp()) { 
    case (OpNum):
    case (OpInt):
      {
        *ub = *ub - coeff*(opc * (n1->getVal()));
      }
      continue;
    case (OpVar):
      //Update the linear part of the function
      {
        VariablePtr v;
        v = problem_->getVariable(n1->getV()->getId());
        if (lf->hasVar(v) == true) {
          lf->incTerm(v,coeff*opc);
        } else {
          lf->addTerm(v,coeff*opc);
        }
      }
      continue;
    case (OpPlus):
      if (itnum[n1->getIndex()] == 0) {
        (*candNodes).push(n1->getL());
        (*candOp).push(opc);
        (*candNodes).push(n1->getR());
        (*candOp).push(opc);
      } else {
        updateVisNodeItr(&itr, &m, n1, opc, itnum[n1->getIndex()],&sNodes,
                         &sOps,&sVars,&sConsts);
      }
      break;
    case (OpMinus):
      if (itnum[n1->getIndex()] == 0) {
        (*candNodes).push(n1->getL());
        (*candOp).push(opc);
        (*candNodes).push(n1->getR());
        (*candOp).push(-1*opc);
      } else {
        updateVisNodeItr(&itr, &m, n1, opc, itnum[n1->getIndex()],&sNodes,
                         &sOps,&sVars,&sConsts);
      }
      break;
    case (OpSumList):
      if (itnum[n1->getIndex()] == 0) {
        for (CNode **it = n1->getListL(); it != n1->getListR(); ++it) {
          n2 = *it;
          (*candNodes).push(n2);
          (*candOp).push(opc);
        } 
      } else {
        updateVisNodeItr(&itr, &m, n1, opc, itnum[n1->getIndex()],&sNodes,
                         &sOps,&sVars,&sConsts);
      }
      break; 
    case (OpUMinus):
      if (itnum[n1->getIndex()] == 0) {
        (*candNodes).push(n1->getL());
        (*candOp).push(opc*(-1));
      } else {
        updateVisNodeItr(&itr, &m, n1, opc, itnum[n1->getIndex()],&sNodes,
                         &sOps,&sVars,&sConsts);
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
    case (OpMult):
    case (OpDiv):
    case (OpPowK):
    case (OpIntDiv):
      {
        bool sep = explore(&itr, n1, &m, opc, itnum, nvar,&sNodes,
                         &sOps,&sVars,&sConsts);
        if (!sep) {
          return false;      
        }
      }
      break;
    case (OpNone):
    case (OpPow):
    default:
      return false;      
    }
  }
  delete [] itnum;

  //Clear sepNodes_ by removing zero entries formed during
  //merge operation
  for (UInt i = 0; i < sepNodes_.size(); ) {
    if (sepNodes_[i].size() == 0) {
      sepOps_.erase(sepOps_.begin()+i);  
      sepVars_.erase(sepVars_.begin()+i);  
      sepNodes_.erase(sepNodes_.begin()+i);  
      sepConst_.erase(sepConst_.begin()+i);  
    } else {
      ++i;
    }
  }

  //Continue separability detection only if searable parts are more than one.
  if (sepNodes_.size() > 0) {
    return true;
  }
  return false; 
}


void TransSep::tempPopulate(CNode *n1, std::stack<CNode *> * tempNodes)
{
  switch (n1->getOp()) {
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
  case (OpSumList):
    for (CNode **it=n1->getListL(); it!=n1->getListR(); ++it) {
      (*tempNodes).push(*it);
    }
    break;
  case (OpNum):
  case (OpInt):
  case (OpVar):
  case (OpPow):
  case (OpNone): 
  default:
    break;
  }
  return;
}


void TransSep::updateVisNodeItr(UInt* j, std::vector<UInt >*m, CNode *n1,
                                int opc, int idx, CNodeQ *sNodes,
                                std::deque<int > *sOps, 
                                std::vector<CNode *> *sVars,
                                std::vector<CNode *> *sConsts)
{
  UInt mNum;                    //mNum for merge operation
  (*sOps).push_back(opc);
  (*sNodes).push_back(n1);
  mNum = mergeItrNum(*j, idx, m);
  mergeItrInfo(mNum,sNodes,sOps,sVars,sConsts);
  return;
}


void TransSep::writeProb()
{
  problem_->calculateSize(true);
  if (true == env_->getOptions()->findBool("display_separable_size")->
      getValue()) {
    problem_->writeSize(logger_->msgStream(LogNone));
  }
  if (true == env_->getOptions()->findBool("display_separable_problem")->
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

