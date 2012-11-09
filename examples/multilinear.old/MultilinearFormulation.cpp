/*
 *     MINOTAUR -- It's only 1/2 bull
 *
 *     (C)opyright 2009-- The MINOTAUR Team
 */


#include <cassert>
#include <iterator>

#include <Types.h>

#include "MultilinearFormulation.h"


using namespace Minotaur;
using namespace std;

#define DEBUG_CONVEX_HULL
#undef DEBUG_ROW_BY_ROW
#undef DEBUG_TERM_BY_TERM

MultilinearFormulation::MultilinearFormulation(InstancePtr instance, RelaxationStrategy s)
{
  originalInstance_ = instance;
  relaxStrategy_ = s;

  switch(relaxStrategy_) {
  case CONVEX_HULL:
    makeConvexHull();
    break;
  case ROW_BY_ROW:
    makeRowByRow();
    break;
  case TERM_BY_TERM:    
    makeTermByTerm();
    break;
  case MCCORMICK:
    makeMcCormick();
    break;
  default:
    assert(0);
  }

}





void 
MultilinearFormulation::allExtreme(vector<int> &S, vector<double> &lb, 
                                   vector<double> &ub, vector<vector<double> > &E)
{
  vector<double> val(S.size());
  int ix = 0;
  visit(S, lb, ub, ix, val, E);    

#if 0
  for(UInt i = 0; i < E.size(); i++) {
    for(UInt j = 0; j < E[i].size(); j++) {
      cout << E[i][j] << ",";
    }
    cout << endl;
  }
#endif


}

void
MultilinearFormulation::boundsOnProduct(ConstVariablePtr x1, ConstVariablePtr x2,
                                        double &lb, double &ub)
{
  double l1 = x1->getLb();
  double u1 = x1->getUb();
  double l2 = x2->getLb();
  double u2 = x2->getUb();

  lb = l1 * l2;
  ub = l1 * l2;

  lb = std::min(lb, u1*l2);
  ub = std::max(ub, u1*l2);

  lb = std::min(lb, u1*u2);
  ub = std::max(ub, u1*u2);

  lb = std::min(lb, l1*u2);
  ub = std::max(ub, l1*u2);

}

void
MultilinearFormulation::makeConvexHull()
{
  
  // First we do some processing of the instance to determine how many 
  //  multilinear or quadratic constraints, and we store the indicies
  // in the instance for later    
  vector<int> lcid;
  vector<int> mlcid;
  for(ConstConstraintIterator it = originalInstance_->consBegin(); 
      it != originalInstance_->consEnd(); ++it) {
    FunctionType ft = (*it)->getFunctionType();
    if (ft == Multilinear) {
      mlcid.push_back((*it)->getId());
    }    
    else if (ft == Bilinear) {
      mlcid.push_back((*it)->getId());
    }
    else if (ft == Linear) {
      lcid.push_back((*it)->getId());
    }
  }
    

  // add x variables
  vector<double> lb;
  vector<double> ub;

  vector<VariablePtr> xvars;
  int nv = 0;
  for (ConstVariableIterator it = originalInstance_->varsBegin(); 
       it != originalInstance_->varsEnd(); ++it) {
    VariablePtr v = *it;
    VariablePtr vnew = VariablePtr(new Variable(nv, v->getLb(), v->getUb(), v->getType()));
    lb.push_back(v->getLb());
    ub.push_back(v->getUb());

    variableMapping_.insert(make_pair(vnew,v));    
    variables_.push_back(vnew);
    xvars.push_back(vnew);
    nv++;
  }

  // Add z vars
  vector<VariablePtr> zvars;
  for(UInt i = 0; i < mlcid.size(); i++) {
    const ConstraintPtr mlc = originalInstance_->getConstraint(mlcid[i]);

    VariablePtr vnew = VariablePtr(new Variable(nv,mlc->getLb(),mlc->getUb(),Continuous));
    variables_.push_back(vnew);
    zvars.push_back(vnew);
    nv++;    
  }

  // Enumerate all extreme points
  int origNumVars = originalInstance_->getNumVars();
  vector<int> S(origNumVars);
  for(int i = 0; i < origNumVars; i++) {
    S[i] = i;
  }
  vector<vector<double> > V;
  allExtreme(S, lb, ub, V);

  // Add lambda variables
  vector<VariablePtr> lambdavars;
  for(UInt i = 0; i < V.size(); i++) {
    VariablePtr vnew = VariablePtr(new Variable(nv,0.0,1.0,Continuous));
    variables_.push_back(vnew);
    lambdavars.push_back(vnew);
    nv++;    
  }    

  // Add the original linear constraints (on x)
  for(int i = 0; i < lcid.size(); i++) {
    const ConstraintPtr mlc = originalInstance_->getConstraint(lcid[i]);
    const LinearFunctionPtr olf = mlc->getLinearFunction();
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
    for(ConstVariableGroupIterator it = olf->varsBegin(); it != olf->varsEnd(); ++it) {
      lf->addTerm(xvars[it->first->getId()], it->second);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, mlc->getLb(), mlc->getUb());
    constraints_.push_back(c);    
#if defined(DEBUG_CONVEX_HULL)
      c->display();
#endif

  }

  // Write x in terms of extreme points
  for(int i = 0; i < origNumVars; i++) {
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
    lf->addTerm(xvars[i], -1.0);
    for(int j = 0; j < V.size(); j++) {
      lf->addTerm(lambdavars[j], V[j][i]);
    }
    //lf->display();
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, 0.0, 0.0);
    constraints_.push_back(c);    
    //XXX Should I set the ID?
#if defined(DEBUG_CONVEX_HULL)
      c->display();
#endif

  }

  // Write z in terms of extreme points
  for(int i = 0; i < mlcid.size(); i++) {
     LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
     lf->addTerm(zvars[i], -1.0);
     const ConstraintPtr mlc = originalInstance_->getConstraint(mlcid[i]);
     const FunctionPtr mlf = mlc->getFunction();

     for(int j = 0; j < V.size(); j++) {
       double zval = mlf->eval(V[j]);
       //cout << "zval = " << zval << endl;
       lf->addTerm(lambdavars[j], zval);
     }
     //lf->display();  cout << endl << endl;     
     FunctionPtr f = (FunctionPtr) new Function(lf);
     ConstraintPtr c = (ConstraintPtr) new Constraint(f, 0.0, 0.0);
     constraints_.push_back(c);    
#if defined(DEBUG_CONVEX_HULL)
      c->display();
#endif

  }

  // Add convexity constraint
  LinearFunctionPtr lf =  LinearFunctionPtr(new LinearFunction());
  for(int j = 0; j < V.size(); j++) {
    lf->addTerm(lambdavars[j], 1.0);
  }
  FunctionPtr f = (FunctionPtr) new Function(lf);
  ConstraintPtr c = (ConstraintPtr) new Constraint(f, 1.0, 1.0);
  constraints_.push_back(c);      

#if defined(DEBUG_CONVEX_HULL)
      c->display();
#endif


  LinearFunctionPtr olf = LinearFunctionPtr(new LinearFunction());

  // Add objective (on x vars only)
  const LinearFunctionPtr originalInstance_olf = originalInstance_->getObjective()->getLinearFunction();
  for (ConstVariableGroupIterator it = originalInstance_olf->varsBegin(); 
       it != originalInstance_olf->varsEnd(); ++it) {
    olf->addTerm(xvars[it->first->getId()], it->second);    
  }
  FunctionPtr of = (FunctionPtr) new Function(olf);
  objective_ = ObjectivePtr(new Objective(of, 0));    

}

void 
MultilinearFormulation::makeMcCormick()
{

  // First we do some processing of the instance to determine how many 
  //  multilinear or quadratic constraints, and we store the indicies
  // in the instance for later    
  vector<int> lcid;
  vector<int> mlcid;
  for(ConstConstraintIterator it = originalInstance_->consBegin(); 
      it != originalInstance_->consEnd(); ++it) {
    FunctionType ft = (*it)->getFunctionType();
    if (ft == Multilinear) {
      mlcid.push_back((*it)->getId());
    }    
    else if (ft == Bilinear) {
      mlcid.push_back((*it)->getId());
    }
    else if (ft == Linear) {
      lcid.push_back((*it)->getId());
    }
  }

  // add x variables
  vector<double> lb;
  vector<double> ub;

  vector<VariablePtr> xvars;
  int nv = 0;
  for (ConstVariableIterator it = originalInstance_->varsBegin(); 
       it != originalInstance_->varsEnd(); ++it) {
    VariablePtr v = *it;
    VariablePtr vnew = VariablePtr(new Variable(nv, v->getLb(), v->getUb(), v->getType()));
    lb.push_back(v->getLb());
    ub.push_back(v->getUb());

    variableMapping_.insert(make_pair(vnew,v));    
    variables_.push_back(vnew);
    xvars.push_back(vnew);
    nv++;
  }

  // Add the linear constraints
  for(int i = 0; i < lcid.size(); i++) {
    const ConstraintPtr mlc = originalInstance_->getConstraint(lcid[i]);
    const LinearFunctionPtr olf = mlc->getLinearFunction();
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
    for(ConstVariableGroupIterator it = olf->varsBegin(); it != olf->varsEnd(); ++it) {
      lf->addTerm(xvars[it->first->getId()], it->second);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, mlc->getLb(), mlc->getUb());
    constraints_.push_back(c);    
  }


  // The w variables
  vector<VariablePtr> wvars;

  // The z vars: These are what you put in the objective
  vector<ConstVariablePtr> zvars;

  // The bilinear terms
  map <VariablePtr, ConstVariablePair> blterms;

  // Go through multilinear rows.  Add constraints, and create maps
  //  for mccormick vars
  for(int i = 0; i < mlcid.size(); i++) {


    const ConstraintPtr omlc = originalInstance_->getConstraint(mlcid[i]);
    const LinearFunctionPtr olf = omlc->getLinearFunction();
    const QuadraticFunctionPtr oqf = omlc->getQuadraticFunction();
    const NonlinearFunctionPtr onlf = omlc->getNonlinearFunction();
    //!!! Don't make this shared by boost, it will get confused in counting
    MultilinearFunction *omlf = dynamic_cast<MultilinearFunction *>(onlf.get());
    
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());

    // Linear part of constraint remains the same
    for(ConstVariableGroupIterator it = olf->varsBegin(); it != olf->varsEnd(); ++it) {
      lf->addTerm(xvars[it->first->getId()], it->second);
    }

    // Quadratic part gets a new variable for every term
    for(ConstVariablePairGroupIterator it = oqf->begin(); it != oqf->end(); ++it) {

      ConstVariablePtr x1 = it->first.first;
      ConstVariablePtr x2 = it->first.second;
      //Bounds on product depend on whether variable bounds are < 0, > 0
      double lb = 0.0;
      double ub = 0.0;
      boundsOnProduct(x1,x2,lb,ub);

      VariablePtr w = VariablePtr(new Variable(nv, lb, ub, Continuous));
      nv++;
      variables_.push_back(w);
      wvars.push_back(w);
      blterms.insert(make_pair(w,ConstVariablePair(x1,x2)));
      lf->addTerm(w,it->second);
    }


    // Multilinear part gets d-1 new vars.
    //  We do mccormick (for now) in the FIFO way
    for(constMultilinearTermContainerIterator it = omlf->termsBegin(); 
        it != omlf->termsEnd(); ++it) {
      
      set<ConstVariablePtr>::const_iterator it2 = it->second.begin();
      ConstVariablePtr x1 = *it2;
      ++it2;
      for( ; it2 != it->second.end(); ++it2) {
        ConstVariablePtr x2 = *it2;

        double lb = 0.0;
        double ub = 0.0;
        boundsOnProduct(x1,x2,lb,ub);

        VariablePtr w = VariablePtr(new Variable(nv, lb, ub, Continuous));
        nv++;
        variables_.push_back(w);
        wvars.push_back(w);
        blterms.insert(make_pair(w,ConstVariablePair(x1,x2)));
        x1 = w;
      }
      // Now x1 is the var you should add to the linear constraint
      lf->addTerm(x1,it->first);
      zvars.push_back(x1);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, omlc->getLb(), omlc->getUb());
    constraints_.push_back(c); 

  }



  // Now add all the constraints for each new bilinear term
  for(map<VariablePtr, ConstVariablePair>::iterator it = blterms.begin();
      it != blterms.end(); ++it) {
    ConstVariablePtr w = it->first;
    ConstVariablePtr x1 = it->second.first;
    ConstVariablePtr x2 = it->second.second;
    LinearFunctionPtr lf1 = LinearFunctionPtr(new LinearFunction());
    LinearFunctionPtr lf2 = LinearFunctionPtr(new LinearFunction());
    LinearFunctionPtr lfw = LinearFunctionPtr(new LinearFunction());

    // Create new lambda variables
    VariablePtr lamll = VariablePtr(new Variable(nv, 0.0, 1.0, Continuous));
    nv++;
    variables_.push_back(lamll);

    VariablePtr lamul = VariablePtr(new Variable(nv, 0.0, 1.0, Continuous));
    nv++;
    variables_.push_back(lamul);

    VariablePtr lamuu = VariablePtr(new Variable(nv, 0.0, 1.0, Continuous));
    nv++;
    variables_.push_back(lamuu);

    VariablePtr lamlu = VariablePtr(new Variable(nv, 0.0, 1.0, Continuous));
    nv++;
    variables_.push_back(lamlu);

    // Just enumerate extreme points yourself
    lf1->addTerm(x1,-1.0);
    lf1->addTerm(lamll, x1->getLb()); 
    lf1->addTerm(lamul, x1->getUb()); 
    lf1->addTerm(lamuu, x1->getUb()); 
    lf1->addTerm(lamlu, x1->getLb()); 

    // Just enumerate extreme points yourself
    lf2->addTerm(x2,-1.0);
    lf2->addTerm(lamll, x2->getLb()); 
    lf2->addTerm(lamul, x2->getLb()); 
    lf2->addTerm(lamuu, x2->getUb()); 
    lf2->addTerm(lamlu, x2->getUb()); 
    
    lfw->addTerm(w, -1.0);
    lfw->addTerm(lamll, x1->getLb()*x2->getLb()); 
    lfw->addTerm(lamul, x1->getUb()*x2->getLb()); 
    lfw->addTerm(lamuu, x1->getUb()*x2->getUb()); 
    lfw->addTerm(lamlu, x1->getLb()*x2->getUb()); 
    
    // Add the x1,x2,and w rows
    FunctionPtr f1 = (FunctionPtr) new Function(lf1);
    ConstraintPtr c1 = (ConstraintPtr) new Constraint(f1, 0.0, 0.0);
    constraints_.push_back(c1);  

    FunctionPtr f2 = (FunctionPtr) new Function(lf2);
    ConstraintPtr c2 = (ConstraintPtr) new Constraint(f2, 0.0, 0.0);
    constraints_.push_back(c2);  

    FunctionPtr fw = (FunctionPtr) new Function(lfw);
    ConstraintPtr cw = (ConstraintPtr) new Constraint(fw, 0.0, 0.0);
    constraints_.push_back(cw);  
    
    // Add the convexity constraint
    LinearFunctionPtr convex_lf =  LinearFunctionPtr(new LinearFunction());
    convex_lf->addTerm(lamll, 1.0);
    convex_lf->addTerm(lamul, 1.0);
    convex_lf->addTerm(lamuu, 1.0);
    convex_lf->addTerm(lamlu, 1.0);

    FunctionPtr convex_f = (FunctionPtr) new Function(convex_lf);
    ConstraintPtr convex_c = (ConstraintPtr) new Constraint(convex_f, 1.0, 1.0);
    constraints_.push_back(convex_c);      
        
  }


  LinearFunctionPtr olf = LinearFunctionPtr(new LinearFunction());
  // Add objective (on x vars only)
  const LinearFunctionPtr originalInstance_olf = originalInstance_->getObjective()->getLinearFunction();
  for (ConstVariableGroupIterator it = originalInstance_olf->varsBegin(); 
       it != originalInstance_olf->varsEnd(); ++it) {
    olf->addTerm(xvars[it->first->getId()], it->second);    
  }
  
  FunctionPtr of = (FunctionPtr) new Function(olf);
  objective_ = ObjectivePtr(new Objective(of, 0));    
  

}

void
MultilinearFormulation::makeRowByRow()
{

  // First we do some processing of the instance to determine how many 
  //  multilinear or quadratic constraints, and we store the indicies
  // for later    
  vector<int> lcid;
  vector<int> mlcid;
  for(ConstConstraintIterator it = originalInstance_->consBegin(); 
      it != originalInstance_->consEnd(); ++it) {
    FunctionType ft = (*it)->getFunctionType();
    if (ft == Multilinear) {
      mlcid.push_back((*it)->getId());
    }    
    else if (ft == Bilinear) {
      mlcid.push_back((*it)->getId());
    }
    else if (ft == Linear) {
      lcid.push_back((*it)->getId());
    }
  }
    
  // add x variables
  vector<double> lb;
  vector<double> ub;

  vector<VariablePtr> xvars;
  int nv = 0;
  for (ConstVariableIterator it = originalInstance_->varsBegin(); 
       it != originalInstance_->varsEnd(); ++it) {
    VariablePtr v = *it;
    VariablePtr vnew = VariablePtr(new Variable(nv, v->getLb(), v->getUb(), v->getType()));
    lb.push_back(v->getLb());
    ub.push_back(v->getUb());

    variableMapping_.insert(make_pair(vnew,v));    
    variables_.push_back(vnew);
    xvars.push_back(vnew);
    nv++;
  }

  // Add the linear constraints
  for(int i = 0; i < lcid.size(); i++) {
    const ConstraintPtr mlc = originalInstance_->getConstraint(lcid[i]);
    const LinearFunctionPtr olf = mlc->getLinearFunction();
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
    for(ConstVariableGroupIterator it = olf->varsBegin(); it != olf->varsEnd(); ++it) {
      lf->addTerm(xvars[it->first->getId()], it->second);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, mlc->getLb(), mlc->getUb());
    constraints_.push_back(c);    
  }

  vector<VariablePtr> zvars;
  vector<vector< VariablePtr> > lambdavars(mlcid.size());  
  for(UInt i = 0; i < mlcid.size(); i++) {
    
    // One 'z' var per row
    VariablePtr zvar = VariablePtr(new Variable(nv, -INFINITY, INFINITY, Continuous));
    variables_.push_back(zvar);
    zvars.push_back(zvar);
    nv++;
    
    // Determine what variables appear nonlinearly...
    const ConstraintPtr mlc = originalInstance_->getConstraint(mlcid[i]);
    vector<int> nlvars = nonlinearVarsInConstraint(mlc);

#if defined(DEBUG_ROW_BY_ROW)
    cout << "Vars in ml constraint " << i << " have ix: ";
    for(vector<int>::iterator it = nlvars.begin(); it != nlvars.end(); ++it) {
      cout << (*it) << " ";
    }
    cout << endl;
#endif

    
    // Add lambda vars for this row
    vector<vector<double> > V;
    allExtreme(nlvars, lb, ub, V);
    for(UInt j = 0; j < V.size(); j++) {
      VariablePtr vnew = VariablePtr(new Variable(nv,0.0,1.0,Continuous));
      variables_.push_back(vnew);
      lambdavars[i].push_back(vnew);
      nv++;
    }

    for(UInt k = 0; k < nlvars.size(); k++) {
      // Create 'x' portion of this constraint
      LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
      lf->addTerm(xvars[nlvars[k]], -1.0);

      for(UInt j = 0; j < V.size(); j++) {
        lf->addTerm(lambdavars[i][j], V[j][k]);
      }                
      
      FunctionPtr f = (FunctionPtr) new Function(lf);
      ConstraintPtr c = (ConstraintPtr) new Constraint(f, 0.0, 0.0);
      constraints_.push_back(c);  

#if defined(DEBUG_ROW_BY_ROW)
      c->display();
#endif
    }
          
    
    // Now add the 'z' var definition
    LinearFunctionPtr tlf = LinearFunctionPtr(new LinearFunction());
    tlf->addTerm(zvar, -1.0);
    const FunctionPtr mlf = mlc->getFunction();
    const LinearFunctionPtr mlf_lf = mlf->getLinearFunction();
    

    for(int j = 0; j < V.size(); j++) {
      //XXX Kludgy, but just create a vector for the (full) point
      int onv = originalInstance_->getNumVars();
      vector<double> xe(onv,0.0);
      for(UInt k = 0; k < nlvars.size(); k++) {
        xe[nlvars[k]] = V[j][k];
      }
      double zval = mlf->eval(xe);
      // Need to subtract off linear part, since yuo keep those variables in
      zval -= mlf_lf->eval(xe);

      tlf->addTerm(lambdavars[i][j], zval);
    }
    FunctionPtr tf = (FunctionPtr) new Function(tlf);
    ConstraintPtr tc = (ConstraintPtr) new Constraint(tf, 0.0, 0.0);
    constraints_.push_back(tc);  

#if defined(DEBUG_ROW_BY_ROW)
      tc->display();
#endif
    
    // Now add the linear constraint involving linear x and t
    LinearFunctionPtr xtlf = LinearFunctionPtr(new LinearFunction());
    for(ConstVariableGroupIterator it = mlf_lf->varsBegin(); it != mlf_lf->varsEnd(); ++it) {
      xtlf->addTerm(xvars[it->first->getId()], it->second);
    }
    xtlf->addTerm(zvar,1.0);
    FunctionPtr xtf = (FunctionPtr) new Function(xtlf);
    ConstraintPtr xtc = (ConstraintPtr) new Constraint(xtf, mlc->getLb(), mlc->getUb());
    constraints_.push_back(xtc);  

#if defined(DEBUG_ROW_BY_ROW)
    xtc->display();
#endif


    // Also add sum (lambda) = 1
    LinearFunctionPtr convex_lf =  LinearFunctionPtr(new LinearFunction());
    for(int j = 0; j < V.size(); j++) {
      convex_lf->addTerm(lambdavars[i][j], 1.0);
    }
    FunctionPtr convex_f = (FunctionPtr) new Function(convex_lf);
    ConstraintPtr convex_c = (ConstraintPtr) new Constraint(convex_f, 1.0, 1.0);
    constraints_.push_back(convex_c);      

#if defined(DEBUG_ROW_BY_ROW)
    convex_c->display();
#endif

    
  }



  LinearFunctionPtr olf = LinearFunctionPtr(new LinearFunction());
  const LinearFunctionPtr originalInstance_olf = originalInstance_->getObjective()->getLinearFunction();
  for (ConstVariableGroupIterator it = originalInstance_olf->varsBegin(); 
       it != originalInstance_olf->varsEnd(); ++it) {
    olf->addTerm(xvars[it->first->getId()], it->second);    
  }
  FunctionPtr of = (FunctionPtr) new Function(olf);
  objective_ = ObjectivePtr(new Objective(of, 0));    
  

}

void
MultilinearFormulation::makeTermByTerm()
{

  // First we do some processing of the instance to determine how many 
  //  multilinear or quadratic constraints, and we store the indicies
  // in the instance for later    
  vector<int> lcid;
  vector<int> mlcid;
  for(ConstConstraintIterator it = originalInstance_->consBegin(); 
      it != originalInstance_->consEnd(); ++it) {
    FunctionType ft = (*it)->getFunctionType();
    if (ft == Multilinear) {
      mlcid.push_back((*it)->getId());
    }    
    else if (ft == Bilinear) {
      mlcid.push_back((*it)->getId());
    }
    else if (ft == Linear) {
      lcid.push_back((*it)->getId());
    }
  }

  // add x variables
  vector<double> lb;
  vector<double> ub;

  vector<VariablePtr> xvars;
  int nv = 0;
  for (ConstVariableIterator it = originalInstance_->varsBegin(); 
       it != originalInstance_->varsEnd(); ++it) {
    VariablePtr v = *it;
    VariablePtr vnew = VariablePtr(new Variable(nv, v->getLb(), v->getUb(), v->getType()));
    lb.push_back(v->getLb());
    ub.push_back(v->getUb());

    variableMapping_.insert(make_pair(vnew,v));    
    variables_.push_back(vnew);
    xvars.push_back(vnew);
    nv++;
  }

  // Add the linear constraints
  for(int i = 0; i < lcid.size(); i++) {
    const ConstraintPtr mlc = originalInstance_->getConstraint(lcid[i]);
    const LinearFunctionPtr olf = mlc->getLinearFunction();
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
    for(ConstVariableGroupIterator it = olf->varsBegin(); it != olf->varsEnd(); ++it) {
      lf->addTerm(xvars[it->first->getId()], it->second);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, mlc->getLb(), mlc->getUb());
    constraints_.push_back(c);    

#if defined(DEBUG_TERM_BY_TERM)
      c->display();
#endif

  }


  // The w variables
  vector<VariablePtr> wvars;
  // This holds a map between the 'w' variable added and indices of x vars in multilinear product
  map <VariablePtr, vector<int> > mlterms;

  // Go through multilinear rows.  Add constraints, and create maps
  for(int i = 0; i < mlcid.size(); i++) {

    const ConstraintPtr omlc = originalInstance_->getConstraint(mlcid[i]);
    const LinearFunctionPtr olf = omlc->getLinearFunction();
    const QuadraticFunctionPtr oqf = omlc->getQuadraticFunction();
    const NonlinearFunctionPtr onlf = omlc->getNonlinearFunction();
    //!!! Don't make this shared by boost, it will get confused in counting
    MultilinearFunction *omlf = dynamic_cast<MultilinearFunction *>(onlf.get());
    
    LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());

    // Linear part of constraint remains the same
    for(ConstVariableGroupIterator it = olf->varsBegin(); it != olf->varsEnd(); ++it) {
      lf->addTerm(xvars[it->first->getId()], it->second);
    }

    // Quadratic part gets a new variable for every term
    for(ConstVariablePairGroupIterator it = oqf->begin(); it != oqf->end(); ++it) {

      vector<int> mlix;
      mlix.push_back(it->first.first->getId());
      mlix.push_back(it->first.second->getId());

      VariablePtr w = VariablePtr(new Variable(nv, -INFINITY,INFINITY, Continuous));
      nv++;
      variables_.push_back(w);
      wvars.push_back(w);
      //XXX Need to store term for evaluation
      mlterms.insert(make_pair(w,mlix));
      lf->addTerm(w,it->second);
    }

    // Multilinear part gets a new var for every term
    for(constMultilinearTermContainerIterator it = omlf->termsBegin(); 
        it != omlf->termsEnd(); ++it) {

      vector<int> mlix;
      for(set<ConstVariablePtr>::const_iterator it2 = it->second.begin();
          it2 != it->second.end(); ++it2) {
        mlix.push_back((*it2)->getId());
      }
      
      VariablePtr w = VariablePtr(new Variable(nv, -INFINITY, INFINITY, Continuous));
      nv++;
      variables_.push_back(w);
      wvars.push_back(w);
      mlterms.insert(make_pair(w,mlix));
    
      lf->addTerm(w,it->first);
    }
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr c = (ConstraintPtr) new Constraint(f, omlc->getLb(), omlc->getUb());
    constraints_.push_back(c); 

#if defined(DEBUG_TERM_BY_TERM)
    c->display();
#endif

  }
  
  // Now add all the constraints for each new bilinear/multilinear term
  for(map<VariablePtr, vector<int> >::iterator it = mlterms.begin();
      it != mlterms.end(); ++it) {

    ConstVariablePtr w = it->first;
    vector<int> &mlix = it->second;

#if defined(DEBUG_TERM_BY_TERM)
    cout << "mlix: ";
    copy(mlix.begin(), mlix.end(), ostream_iterator<int>(cout, " "));
    cout << endl;
#endif

    // Enumerate extreme points
    vector<vector<double> > V;
    vector<VariablePtr> lambdavars;
    allExtreme(mlix, lb, ub, V);

    // Add lambda vars
    for(UInt j = 0; j < V.size(); j++) {
      VariablePtr vnew = VariablePtr(new Variable(nv,0.0,1.0,Continuous));
      variables_.push_back(vnew);
      nv++;
      lambdavars.push_back(vnew);
    }

    // Write x as convex combination of lambda (for each component)
    for(UInt k = 0; k < mlix.size(); k++) {
      LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
      lf->addTerm(xvars[mlix[k]], -1.0);

      for(UInt j = 0; j < V.size(); j++) {
        lf->addTerm(lambdavars[j], V[j][k]);
      }                
      
      FunctionPtr f = (FunctionPtr) new Function(lf);
      ConstraintPtr c = (ConstraintPtr) new Constraint(f, 0.0, 0.0);
      constraints_.push_back(c);  

#if defined(DEBUG_TERM_BY_TERM)
      c->display();
#endif
    }

    // Write w (term) as convex combination of function values at extreme points
    LinearFunctionPtr wlf = LinearFunctionPtr(new LinearFunction());
    wlf->addTerm(w, -1.0);
    
    for(int j = 0; j < V.size(); j++) {
      // Evaluation at extreme point is just the product
      double product = 1.0;
      for(int k = 0; k < V[j].size(); k++) {
        product *= V[j][k];
      }
      if (product > 1.0e-9 || product < -1.0e-9) {
        wlf->addTerm(lambdavars[j], product);
      }
    }
    FunctionPtr wf = (FunctionPtr) new Function(wlf);
    ConstraintPtr wc = (ConstraintPtr) new Constraint(wf, 0.0, 0.0);
    constraints_.push_back(wc);  

#if defined(DEBUG_TERM_BY_TERM)
    wc->display();
#endif
  
    // Also add sum (lambda) = 1
    LinearFunctionPtr convex_lf =  LinearFunctionPtr(new LinearFunction());
    for(int j = 0; j < V.size(); j++) {
      convex_lf->addTerm(lambdavars[j], 1.0);
    }
    FunctionPtr convex_f = (FunctionPtr) new Function(convex_lf);
    ConstraintPtr convex_c = (ConstraintPtr) new Constraint(convex_f, 1.0, 1.0);
    constraints_.push_back(convex_c);      

#if defined(DEBUG_TERM_BY_TERM)
    convex_c->display();
#endif

  }


  LinearFunctionPtr olf = LinearFunctionPtr(new LinearFunction());
  const LinearFunctionPtr originalInstance_olf = originalInstance_->getObjective()->getLinearFunction();
  for (ConstVariableGroupIterator it = originalInstance_olf->varsBegin(); 
       it != originalInstance_olf->varsEnd(); ++it) {
    olf->addTerm(xvars[it->first->getId()], it->second);    
  }
  FunctionPtr of = (FunctionPtr) new Function(olf);
  objective_ = ObjectivePtr(new Objective(of, 0));    


}

vector<int> 
MultilinearFormulation::nonlinearVarsInConstraint(const ConstraintPtr c)
{
  set<int> ix;
  const QuadraticFunctionPtr qf = c->getQuadraticFunction();
  const NonlinearFunctionPtr nlf = c->getNonlinearFunction();
  const MultilinearFunction *mlf = dynamic_cast<MultilinearFunction *>(nlf.get());
  
  for(ConstVariablePairGroupIterator it = qf->begin(); it != qf->end(); ++it) {
    ix.insert(it->first.first->getId());
    ix.insert(it->first.second->getId());    
  }

  for(constMultilinearTermContainerIterator it = mlf->termsBegin(); 
      it != mlf->termsEnd();
      ++it) {
    for(set<ConstVariablePtr>::const_iterator it2 = it->second.begin();
        it2 != it->second.end(); ++it2) {
      ix.insert((*it2)->getId());
    }
  }

  vector<int> vix;
  for(set<int>::const_iterator it = ix.begin(); it != ix.end(); ++it) {
    vix.push_back(*it);
  }
  return vix;
  
}

void 
MultilinearFormulation::visit(vector<int> &S, vector<double> &lb, vector<double> &ub, 
                              int ix, vector<double> &val, vector<vector<double> > &E) 
{ 
  if (ix == S.size()) {
    E.push_back(val);
    return ;
  }  
  val[ix] = lb[S[ix]];
  visit(S,lb,ub,ix+1,val,E);

  val[ix] = ub[S[ix]];
  visit(S,lb,ub,ix+1,val,E);
  
}

// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
