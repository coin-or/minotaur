/*
 * MIQCP.cpp
 *
 *  Created on: Jun 26, 2014
 *      Author: hongbodong
 */

#include "MIQCP.h"
#include "CDlogdet.h"
//#include <boost/lexical_cast.hpp>

#include "AMPLInterface.h"
#undef filename

using namespace Minotaur;
using namespace MINOTAUR_AMPL;

Double QuadCons::evalLenv(ConstVariablePtr x, Double val)
{
  Double Lb, Ub;
  if (x->getLb()<=val && val<=x->getUb())
  {
    if (x->getType()==Continuous)
    {
      //std::cout<<"continuous. val="<<val;
      return (val*val);
    }
    else if (x->getType()==Binary  ||
             x->getType()==Integer ||
             x->getType()==ImplBin ||
             x->getType()==ImplInt   )
    {
    Lb = std::max(floor(val), x->getLb());
    Ub = std::min(ceil(val), x->getUb());
    return ((Ub+Lb)*val-Ub*Lb);
    }
    else
    {
      std::cout<<"Warning: evalUenv: Unknown variable type."<<std::endl;
      return INFINITY;
    }
  }
  else
  {
    std::cout<<"Warning: evalUenv: value out of bound."<<std::endl;
    return INFINITY;
  }
}

// Evaluate upper envelope of x^2
Double QuadCons::evalUenv(ConstVariablePtr x, Double val)
{
  Double Lb = x->getLb();
  Double Ub = x->getUb();
  if (Lb<=val && val<=Ub)
  {
    return ((Ub+Lb)*val-Ub*Lb);
  }
  else
  {
    std::cout<<"Warning: evalUenv: value out of bound."<<std::endl;
    return -INFINITY;
  }
}

QuadCons::QuadCons(ProblemPtr p, FunctionPtr f, Double rhs)
{
  QuadCons(p, f, rhs, "unnamed");
}

QuadCons::~QuadCons()
{
  //Empty.
}

QuadCons::QuadCons(ProblemPtr p,  FunctionPtr f, Double rhs, std::string name)
{
  _prob = p;
  _f   = f;
  _rhs = rhs;
  _name = name;

  QuadraticFunctionPtr qf = _f->getQuadraticFunction();
  UInt nvars = qf->getNumVars();

  EigenCalculator *eigCalc = new EigenCalculator();

  _eigs = eigCalc->findVectors((ConstQuadraticFunctionPtr) qf);

  delete eigCalc;

  // Set the min and max eigenvalue, and _HessScale
  _mineig = INFINITY;
  _maxeig = -INFINITY;
  for (EigenPairConstIterator eigp = _eigs->begin(); eigp != _eigs->end(); eigp++)
  {
    _mineig = fmin(_mineig, (*eigp).first);
    _maxeig = fmax(_maxeig, (*eigp).first);
  }
  _HessScale = fmax(fabs(_mineig), fabs(_maxeig));
  std::cout<<"HessScaleFactor (2norm) = "<<_HessScale<<std::endl;

  // Now we start to fill _HessPtr, _initd and _InvHessPlusInitd
  _HessScaledPtr = (DoubleArray) new Double[nvars*nvars];
  _initScaled = (DoubleArray) new Double[nvars];
  _InvHessPlusInitd = (DoubleArray) new Double[nvars*nvars];
  for (UInt i=0; i<nvars*nvars; i++)
  {
    if (i<nvars)
    {
      _initScaled[i] = 0.0;
    }
    _HessScaledPtr[i] = 0.0;
    _InvHessPlusInitd[i] = 0.0;
  }

  //UInt seq = 1;
  UInt seq = 0;
  for (VarIntMapConstIterator varintpair=qf->varsBegin();
      varintpair != qf->varsEnd(); varintpair++, seq++)
  {
    _order.insert(std::pair<ConstVariablePtr, UInt>((*varintpair).first, seq));
  }

  ConstVariablePtr var1, var2;
  UInt idx1, idx2;
  Double weight;
  for (VariablePairGroupConstIterator vpg = qf->begin();
      vpg != qf->end(); vpg++)
  {
    var1 = ((*vpg).first).first;
    idx1 = (_order.find(var1))->second;
    var2 = ((*vpg).first).second;
    idx2 = (_order.find(var2))->second;
    weight = (*vpg).second;
    if (idx1==idx2)
      _HessScaledPtr[nvars*(idx1)+idx2] = weight/_HessScale;
    else
    {
      _HessScaledPtr[nvars*(idx1)+idx2] = weight/(2*_HessScale);
      _HessScaledPtr[nvars*(idx2)+idx1] = weight/(2*_HessScale);
    }
  }

  // Now set the initial d
  //Double val_initd = -1.1*fmin(0, _mineig)/_HessScale;
  Double val_initd = -1.5*fmin(0, _mineig)/_HessScale;
  for (UInt i=0; i<nvars; i++)
  {
    _initScaled[i] = val_initd;
  }

  // Compute the initial V, put in _InvHessPlusInitd, only lower triangular part
  cblas_dcopy(nvars*nvars, _HessScaledPtr.get(), 1, _InvHessPlusInitd.get(), 1);
  cblas_daxpy(nvars, 1, _initScaled.get(), 1, _InvHessPlusInitd.get(), nvars+1);

  char UPLO = 'L';
  int LDA = nvars;
  int INFO = 0;
  int n = (int) nvars;
  dpotrf_(&UPLO, &n, _InvHessPlusInitd.get(), &LDA, &INFO);
  dpotri_(&UPLO, &n, _InvHessPlusInitd.get(), &LDA, &INFO);
  //printMat(_InvHessPlusInitd.get(), nvars);
}

std::pair<FunctionPtr, Double> QuadCons::genCvxQuad(CutMethod mtd, ConstSolutionPtr sol,  ConstVarMapPtr varmap)
{
  //std::cout<<"Entering QuadCons::genCvxQuad"<<std::endl;

  FunctionPtr f;
  Double UB;
  QuadraticFunctionPtr qf = _f->getQuadraticFunction();
  LinearFunctionPtr lf = _f->getLinearFunction();
  if (NULL == lf)
  {
    lf = (LinearFunctionPtr) new LinearFunction();
  }
  UInt n = qf->getNumVars();

  // For each mode, prepare function f and upper bound UB
  if (mtd==InitialRelax)
  {
    QuadraticFunctionPtr newqf = qf->clone();
    LinearFunctionPtr newlf = lf->clone();

    Double newub = _rhs;
    for (VarIntMapConstIterator varintpair = newqf->varsBegin(); varintpair != newqf->varsEnd(); varintpair++)
    {
      ConstVariablePtr var = (varintpair)->first;

      ConstVariablePtr sqrvar = varmap->getSqrFromRelax(var);

      UInt indx = (_order.find(var))->second;
      //Double dval = _initScaled[indx]*_HessScale;
      Double dval = _initScaled[indx]*_HessScale;
      newqf->incTerm(var, var, dval);
      newlf->incTerm(sqrvar, -dval);
    }
    f = (FunctionPtr) new Function(newlf, newqf);
    UB = newub;
  }
  else if (mtd==Perturbe)
  {
    const Double* psol = sol->getPrimal();
    //boost::scoped_array<Double> wneg(new Double[n]);
    //boost::scoped_array<Double> wpos(new Double[n]);
    //boost::scoped_array<Double> d(new Double[n]);

    // Initialized vectors
    std::vector<Double> wneg(n, 0.0);
    std::vector<Double> wpos(n, 0.0);
    std::vector<Double> d(n, 0.0);

    Double perturbfactor = 0.0;
    Double maxwpos = 0.0;
    for (VarIntMapConstIterator varintpair=qf->varsBegin();
        varintpair != qf->varsEnd(); varintpair++)
    {
      // Variable Pointer
      ConstVariablePtr var = varintpair->first;

      // Original variable pointer
      ConstVariablePtr orivar = varmap->getOriFromRelax(var);

      // Index in _HessScaledPtr, 0 ~ n-1
      UInt indx = (_order.find(var))->second;

      // Solution value
      Double dx = psol[var->getIndex()];

      // Set wneg and wpos
      wneg[indx] = QuadCons::evalLenv((ConstVariablePtr) orivar, dx) - dx*dx;
      wpos[indx] = QuadCons::evalUenv((ConstVariablePtr) orivar, dx) - dx*dx;

      //myprint
      //std::cout<<orivar->getName();
      //PRINT("  neg: %f, pos: %f \n", wneg[indx],wpos[indx]);

      // Set initial d
      //d[indx] = 1.5*std::abs(std::min(0.0, _mineig))/_HessScale;
      d[indx] = _initScaled[indx];

      perturbfactor += wpos[indx];
      maxwpos = fmax(wpos[indx],maxwpos);
    }
    perturbfactor /= (n);

    std::vector<Double> tmp(n,0.0);

    //Adjust wpos
    for (UInt i=0; i<n; i++)
    {
      wneg[i] /= maxwpos;
      wpos[i] /= maxwpos;
      wpos[i] += 0.1*perturbfactor/maxwpos;

      tmp[i] = wpos[i]/_InvHessPlusInitd[(n+1)*i];
      if(PRINTLEVEL)
      {
        std::cout<<"wpos["<<i<<"]:"<<wpos[i]<<" diag_inv_qd: wpos./diag_invQd"<<_InvHessPlusInitd[(n+1)*i]<<std::endl;
      }
    }

    std::sort (tmp.begin(), tmp.end());

    //Double sigma = 100;
    //Double sigma = 1.0396;
    Double sigma = tmp[floor(n/2)-1];
    //std::cout<<"sigma="<<sigma<<std::endl;
    // Compute recommended initial sigma
    //_initSigma = median(wpos./diag(_InvHess));
    //Double sigma = 10.396;
    Int maxIter = 500*n;

    //datacheck(qf->getNumVars(), _HessScaledPtr, wneg.get(), wpos.get(), sigma, maxIter, d.get(), NULL, NULL, NULL);

    if (PRINTLEVEL)
    {
    	printVec(wpos.data(),n);
    	printVec(wneg.data(),n);
    	printVec(d.data(),n);
    	printVec(tmp.data(),n);
    	std::cout<<std::endl;
    	printMat(_HessScaledPtr.get(), n);
    	printMat(_InvHessPlusInitd.get(), n);
    }

    CDlogdet_nonsmooth(n, _HessScaledPtr.get(), wneg.data(), wpos.data(), sigma,
                       maxIter, d.data(), _InvHessPlusInitd.get(),
                       NULL, NULL, NULL);
//    printVec(d.data(),n);

    // Now construct the relaxed constraint f(x) <= UB
    QuadraticFunctionPtr newqf = cloneQuadF();
    LinearFunctionPtr newlf=cloneLinF();
    UB = getRHS();

    for (VarIntMapConstIterator varintpair = newqf->varsBegin(); varintpair != newqf->varsEnd(); varintpair++)
    {
      ConstVariablePtr var = (varintpair)->first;

      // Index in _HessScaledPtr
      UInt idx = (_order.find(var))->second;
      //idx--;
      Double d_scaled = d[idx]*_HessScale;

      ConstVariablePtr sqrvar = varmap->getSqrFromRelax(var);
      newqf->incTerm(var, var, d_scaled);
      newlf->incTerm(sqrvar, -d_scaled);

      if (PRINTLEVEL)
      {
    	std::cout<<var->getName()<<" new d: "<<d_scaled<<std::endl;
      }
    }

    f = (FunctionPtr) new Function(newlf, newqf);

    //myprint
    //f->write(std::cout);
  }
  else
  {
    std::cout<<"QuadCons::genCvxQuad() : CutMethod not recognized."<<std::endl;
  }

  return std::pair<FunctionPtr, Double>(f,UB);
}

Double QuadCons::getMinEig()
{
  return _mineig;
}

QuadraticFunctionPtr QuadCons::cloneQuadF()
{
  QuadraticFunctionPtr qf = (_f->getQuadraticFunction())->clone();
  return qf;
}

LinearFunctionPtr QuadCons::cloneLinF()
{
  LinearFunctionPtr lf;
  if (NULL != _f->getLinearFunction())
  {
    lf = (_f->getLinearFunction())->clone();
  }
  else
  {
    lf = (LinearFunctionPtr) new LinearFunction();
  }
  return lf;
}

bool QuadCons::hasVarInQuad(ConstVariablePtr var) const
{
  return ((_f->getQuadraticFunction())->hasVar(var));
}


VarMap::VarMap()
{
  //Empty
}


ConstVariablePtr VarMap::getSqrFromRelax(ConstVariablePtr relax) const
{
  ConstVariablePtr sqrvar;
  sqrvar = (_relax2sqr.find(relax))->second;
  return sqrvar;
}

ConstVariablePtr VarMap::getOriFromRelax(ConstVariablePtr relax) const
{
  ConstVariablePtr ori;
  ori = (_relax2ori.find(relax))->second;
  return ori;
}

void VarMap::LinkOriRelax(ConstVariablePtr ori, ConstVariablePtr relax)
{
  _ori2relax.insert(std::pair<ConstVariablePtr, ConstVariablePtr>(ori,relax));
  _relax2ori.insert(std::pair<ConstVariablePtr, ConstVariablePtr>(relax,ori));
}

void VarMap::LinkRelaxSqr(ConstVariablePtr relax, ConstVariablePtr sqrvar)
{
  _relax2sqr.insert(std::pair<ConstVariablePtr, ConstVariablePtr>(relax,sqrvar));
}

void VarMap::write(std::ostream& out) const
{
  out<<"===================VarMap=================="<<std::endl;
  out<<"Original variables <==> Relaxed variables"<<std::endl;
  for (VarVarMap::const_iterator it = _ori2relax.begin(); it != _ori2relax.end();
      it ++)
  {
    VariablePtr p1 = (it->first);
    VariablePtr p2 = (it->second);
    out<<"[id"<<p1->getId()<<"] "<<p1->getName() <<" at "<<(void*) p1.get()<<" <==> "
        <<"[id"<<p2->getId()<<"] "<<p2->getName()<<" at "<<(void*) p2.get()<<std::endl;
  }
  out<<"Relaxed variables <==> Lifted variables"<<std::endl;
  for (VarVarMap::const_iterator it = _relax2sqr.begin(); it != _relax2sqr.end();
      it ++)
  {
    VariablePtr p1 = (it->first);
    VariablePtr p2 = (it->second);
    out<<"[id"<<p1->getId()<<"] "<<p1->getName() <<" at "<<(void*) p1.get()<<" <==> "
        <<"[id"<<p2->getId()<<"] "<<p2->getName()<<" at "<<(void*) p2.get()<<std::endl;
  }
  out<<"===================VarMap=End=============="<<std::endl;
}

MIQCP::MIQCP(MINLPType minlptype, std::string filein)
{
  _me = "sdp-cuts: ";
  /// Call corresponding initialization
  switch (minlptype)
  {
  case ProbTypeBoxQP:
    _type = ProbTypeBoxQP;
    init_boxqp(filein);
    break;
  case AMPLstubInput:
    _type = AMPLstubInput;
    init_ampl(filein);
    //return;
    break;
  default:
    std::cout<<"Warning: undefined MINLPType"<<std::endl;
    break;
  }

  // Creating lifting and initial Relaxation of all nonconvex quadratic constraints.
  initRelax();

  _varmap->write(std::cout);

  _prob->setNativeDer();

  //std::cout<<"Relaxed Problem:"<<std::endl;
  //_prob->write(std::cout);

  // Initialize engine
  _engine = (FilterSQPEnginePtr) new FilterSQPEngine(_env);
  _engine->load(_prob);

  // First solve
  _engstatus = _engine->solve();
  _sol = _engine->getSolution();

  std::cout<<"Initial relaxation solve: engine status = "<<_engstatus<<std::endl;
  writeSol(false);
  //_sol->write(std::cout);
}

//MIQCP::init_ampl(ProblemPtr otherprob )
void MIQCP::init_ampl(std::string filein)
{
  int err = 0;

  _env = (EnvPtr) new Environment();
  AMPLInterfacePtr iface = (AMPLInterfacePtr) new AMPLInterface(_env);
  _orig_prob = iface->readInstance(filein);

  QuadraticFunctionPtr qf;
  LinearFunctionPtr lf, conslf;
  FunctionPtr f, consf;
  VariablePtr quadv;
  ConstraintPtr pcons;

  // Linearize objective
  ObjectivePtr pobj = (ObjectivePtr) _orig_prob->getObjective();
  switch (pobj->getFunctionType())
  {
  case Constant:
    break; //Nothing to be done
  case Linear:
    break; //Nothing to be done
  case Bilinear:
  case Quadratic:
    //std::cout<<"Original objective:"<<std::endl;
    //pobj->write(std::cout);

    // Get linear part of the objective function
    lf = (LinearFunctionPtr) pobj->getLinearFunction();
    if (NULL==lf)
    {
      lf = (LinearFunctionPtr) new LinearFunction();
    }
    else
    {
      lf = lf->clone();
    }
    // New Variable to represent quadratic part
    quadv = _orig_prob->newVariable(-INFINITY, INFINITY, Continuous, "quadobj");
    lf->addTerm(quadv, 1.0);
    f = (FunctionPtr) new Function(lf); // new objective

    // Put the quadratic objective into the constraints
    qf = (QuadraticFunctionPtr) (pobj->getQuadraticFunction())->clone();
    conslf = (LinearFunctionPtr) new LinearFunction();
    conslf->addTerm(quadv, -1.0);
    consf = (FunctionPtr) new Function(conslf, qf);
    if (Minimize==pobj->getObjectiveType())
    {
      pcons=_orig_prob->newConstraint(consf, -INFINITY, 0, "linObj");
      std::cout<<"Sense is Minimize"<<std::endl;
    }
    else
    {
      pcons=_orig_prob->newConstraint(consf, 0, INFINITY, "linObj");
      std::cout<<"Sense is Maximize"<<std::endl;
    }

    //std::cout<<"Constraint added:"<<std::endl;
    //pcons->write(std::cout);

    _orig_prob->changeObj(f, pobj->getConstant());
    std::cout<<"(MIQCP::init_ampl) Objective linearized"<<std::endl;

    //std::cout<<"New objective:"<<std::endl;
    //(_orig_prob->getObjective())->write(std::cout);

    break;
  case Multilinear:
  case Nonlinear:
  case Polynomial:
  case UnknownFunction:
    std::cerr<<"Objective function not yet supported!"<<std::endl;
    break;
  }

  //_orig_prob ->write(std::cout);
  _varmap = (VarMapPtr) new VarMap();
  _env->startTimer(err);
  _sol = (ConstSolutionPtr) new Solution();

  // Clone the problem, make variable links
  _prob = _orig_prob->clone();
  for (VariableConstIterator it = _orig_prob->varsBegin();
       it != _orig_prob->varsEnd(); it++)
  {
    UInt id = (*it)->getId();
    _varmap->LinkOriRelax((*it), _prob->getVariable(id));
  }

  Double effLB, effUB;
  // Relax all the variables to continuous in _prob
  for (VariableConstIterator it = _prob->varsBegin();
       it != _prob->varsEnd(); it++)
  {
    if (Binary==(*it)->getType() )
    {
      effLB = 0.0;
      effUB = 1.0;
      _prob->setVarType(*it, Continuous);
      _prob->changeBound(*it, effLB, effUB);
      std::cout<<"Bin. Variable relaxed: "<<(*it)->getName()<<std::endl;
    }
    else if(Integer == (*it)->getType())
    {
      effLB = ceil((*it)->getLb());
      effUB = floor((*it)->getUb());
      _prob->setVarType(*it, Continuous);
      _prob->changeBound(*it, effLB, effUB);
      std::cout<<"Int. Variable relaxed: "<<(*it)->getName()<<std::endl;
    }
    else if(Continuous == (*it)->getType())
    {
      std::cout<<"Continuous Variable: "<<(*it)->getName()<<std::endl;
    }
    else
    {
      std::cerr<<"(MIQCP::init_ampl) Strange variable type? "<<(*it)->getName()<<std::endl;
    }
  }

  EigenCalculator *eigCalc = new EigenCalculator();



  std::vector<ConstraintPtr> tosplit;

  // First part to split all two-sided nonconvex quadratic constraints

  // Record the constraints to split
  for (ConstraintConstIterator it = _prob->consBegin();
      it != _prob->consEnd(); it++)
  {
    // If quadratic
    if (Quadratic==(*it)->getFunctionType() ||
        Bilinear == (*it)->getFunctionType())
    {
      if (-INFINITY != (*it)->getLb() && INFINITY != (*it)->getUb())
      {
        tosplit.push_back(*it);
      }
    }
  }

  QuadraticFunctionPtr qqf;
  LinearFunctionPtr llf;
  FunctionPtr ff;
  // Split them
  for (std::vector<ConstraintPtr>::const_iterator it=tosplit.begin();
       it != tosplit.end(); it++)
  {
    Double LHS = (*it)->getLb();
    qqf = ((*it)->getQuadraticFunction())->clone();
    llf = (*it)->getLinearFunction();
    if (NULL==llf)
    {
      llf = (LinearFunctionPtr) new LinearFunction();
      std::cerr<<"(MIQCP::init_ampl) Warning: NULL==llf"<<std::endl;
    }
    else
    {
      llf = llf->clone();
    }

    ff = (FunctionPtr) new Function(llf, qqf);
    //ff->write(std::cout); std::cout<<std::endl;
    //(*ff) *= (-1);
    ff->operator*=(-1);
    //ff->write(std::cout); std::cout<<std::endl;

    ConstraintPtr newcons = _prob->newConstraint(ff, -INFINITY, -LHS,
        ((*it)->getName()+"-split"));
    std::cout<<"Split 2-sided constraint:"<<std::endl;
    //std::cout<<"Original constraint:"<<std::endl;
    //(*it)->write(std::cout);

    _prob->changeBound(*it, Lower, -INFINITY);
    std::cout<<"Constraint " <<(*it)->getName()<<" splitted."<<std::endl;
    //std::cout<<"Split constraint 1:"<<std::endl;
    //(*it)->write(std::cout);
    //std::cout<<"Split constraint 2:"<<std::endl;
    //newcons->write(std::cout);

    //_prob->markDelete(*it);
  }

  //_prob->delMarkedCons();

  // Now Enumerate over all constraints
  // take all nonconvex quadratic inequalities and put in _ncvxRows
  std::vector<ConstraintPtr> toremove;
  for (ConstraintConstIterator it = _prob->consBegin();
      it != _prob->consEnd(); it++)
  {
    // If quadratic
    if (Quadratic==(*it)->getFunctionType() ||
        Bilinear == (*it)->getFunctionType())
    {
      if (-INFINITY != (*it)->getLb())
      {
        if (INFINITY == (*it)->getUb() )
        {
          //revert
          _prob->reverseSense(*it);
        }
        else
        {
          std::cerr<<"(MIQCP::init_ampl) Error: This should not happen."
            <<std::endl;
        }
      }
      // Process the nonconvex quadratic constraint
      f = (*it)->getFunction();
      qf = f->getQuadraticFunction();

      EigenPtr eigs = eigCalc->findVectors((ConstQuadraticFunctionPtr) qf);

      // If negative eigenvalues exists, mark for removal and push into _ncvxRows
      if (0 < eigs->numNegative())
      {
        QuadConsPtr quadrow = (QuadConsPtr) new QuadCons(_prob, f, (*it)->getUb(), (*it)->getName());
        _ncvxRows.push_back(quadrow);
        toremove.push_back(*it);
        std::cout<<"Quad Constraint"<<(*it)->getName()<<" is nonconvex. Push into _ncvxrow."<<std::endl;
      }
      else
      {
        std::cout<<"Quad Constraint"<<(*it)->getName()<<" is convex. Left as is."<<std::endl;
      }
    }
    else if(Multilinear==(*it)->getFunctionType() ||
        Nonlinear == (*it)->getFunctionType()     ||
        Polynomial ==(*it)->getFunctionType()     ||
        UnknownFunction ==(*it)->getFunctionType()  )
    {
      std::cerr<<"(MIQCP::init_ampl) Warning: non-quadratic nonlinear functions exist. "
          <<"Make sure they are convex!"<<std::endl;
      break;
    }
  }

  for (ConstraintConstIterator it = toremove.begin(); it!=toremove.end();
       it++)
  {
    _prob->markDelete((*it));
    std::cout<<"Constraint " << (*it)->getName() <<" is marked for deletion."<<std::endl;
  }
  _prob->delMarkedCons();

  //std::cout<<"Problem before relaxations added."<<std::endl;
  //_prob->write(std::cout);

  delete eigCalc;
//    ProblemPtr original_inst = (ProblemPtr) new Problem();
//
//    AMPLInterfacePtr iface = (AMPLInterfacePtr) new AMPLInterface(penv);
//    original_inst = iface->readInstance(filein);
//    original_inst->write(std::cout);

//  std::cout<<"MIQCP constructor."<<std::endl;
//  _env = (EnvPtr) new Environment();
//  _prob = otherprob->clone();
//
//  for (VariableConstIterator it = _prob->varsBegin(); it!= _prob->varsEnd(); it++)
//  {
//    VariablePtr var = (*it);
//    if (var->getType()==Binary)
//    {
//      //_prob->setVarType(var, ImplBin);
//      _prob->setVarType(var, Continuous);
//    }
//    else if (var->getType()==Integer)
//    {
//      //_prob->setVarType(var, ImplInt);
//      _prob->setVarType(var, Continuous);
//    }
//  }
//  //_prob->write(std::cout);
  _prob->setNativeDer();
}

// Establish _prob, _orig_prob. Initialize _env.
// Put nonconvex quad cons in _ncvxRows,
// Establish _varmap
void MIQCP::init_boxqp(std::string fname)
{
  int err = 0;
  _env = (EnvPtr) new Environment();

  _orig_prob = (ProblemPtr) new Problem();
  _prob = (ProblemPtr) new Problem();
  _env->startTimer(err);
  _varmap = (VarMapPtr) new VarMap();
  _sol = (ConstSolutionPtr) new Solution();

  std::ifstream fs;
  UInt nvars = 0;
  Double val;
  FunctionPtr f;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  VariablePtr v;

  FunctionPtr ori_f;
  LinearFunctionPtr ori_lf;
  QuadraticFunctionPtr ori_qf;
  VariablePtr ori_v;

  VariablePtr tmpvar1;
  VariablePtr tmpvar2;

  ConstSolutionPtr sol;

  fs.open(fname.c_str(), std::ios::in);

  // Define the 'v' variable
  ori_v = _orig_prob->newVariable(-INFINITY, INFINITY, Continuous, "v");
  v = _prob->newVariable(-INFINITY, INFINITY, Continuous, "v");
  _varmap->LinkOriRelax(ori_v, v);

  fs >> nvars;
  //std::cout<<"Number of variables:"<<(nvars+1)<<std::endl;

  std::string varname;
  for (UInt i=0; i<nvars; ++i) {
    tmpvar1 = _orig_prob->newVariable(0.0, 1.0, Continuous);
    tmpvar2 = _prob->newVariable(0.0, 1.0, Continuous);
    _varmap->LinkOriRelax(tmpvar1, tmpvar2);
  }

  // Load the objective: min v + c'*x
  lf = (LinearFunctionPtr) new LinearFunction();
  ori_lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(v, 1.0);
  ori_lf->addTerm(ori_v, 1.0);
  for (UInt i=1; i<=nvars; i++)
  {
    fs >> val;
    lf->addTerm(_prob->getVariable(i), -val);
    ori_lf->addTerm(_orig_prob->getVariable(i), -val);
  }

  f = (FunctionPtr) new Function(lf);
  ori_f = (FunctionPtr) new Function(ori_lf);
  _prob->newObjective(f, 0.0, Minimize);
  _orig_prob->newObjective(ori_f, 0.0, Minimize);

  // Put the quadratic constraint v >= x'*Q*x into _ncvxRows
  qf = (QuadraticFunctionPtr) new QuadraticFunction();
  ori_qf = (QuadraticFunctionPtr) new QuadraticFunction();

  for (UInt i=1; i<=nvars; i++)
  {
    for (UInt j=1; j<=nvars; j++)
    {
      fs >> val;
      qf->incTerm(_prob->getVariable(i), _prob->getVariable(j), -0.5*val);
      ori_qf->incTerm(_orig_prob->getVariable(i), _orig_prob->getVariable(j), -0.5*val);
    }
  }

  lf = (LinearFunctionPtr) new LinearFunction();
  ori_lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(v, -1.0);
  ori_lf->addTerm(ori_v, -1.0);

  f = (FunctionPtr) new Function(lf, qf);
  ori_f = (FunctionPtr) new Function(ori_lf, ori_qf);

  QuadConsPtr quadrow = (QuadConsPtr) new QuadCons(_prob, f, 0.0, "v>=xt*Q*x");
  _ncvxRows.push_back(quadrow);

  _orig_prob->newConstraint(ori_f, -INFINITY, 0.0, "x'*Q*x-v<=0");
  //_prob->write(std::cout);
  fs.close();
}

// Add 2dimensional convex hull constraint, use variable type in the original variable
// while bounds in the relaxed variable.
void MIQCP::add2dimCvxHull(ConstVariablePtr x, ConstVariablePtr xsqr)
{
  //std::cout<<"Empty function. Here we need original variable type!"<<std::endl;
  ConstVariablePtr ori = _varmap->getOriFromRelax(x);
  Double LB = x->getLb();
  Double UB = x->getUb();

  Double effLB, effUB;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  FunctionPtr f;

  std::cout<<"(MIQCP::add2dimCvxHull) Process "<<ori->getName()<<std::endl;
  switch(ori->getType())
  {
  case Binary:
    if (LB!=0.0 || UB!=1.0)
    {
      std::cout<<"MIQCP::add2dimCvxHull() Warning: Binary vars with non-0/1 bounds."
         <<std::endl;
    }
    else
    {
      LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(x, 1.0);
      lf->addTerm(xsqr, -1.0);

      FunctionPtr f = (FunctionPtr) new Function(lf);
      // Linear equality constraint: x = x^2
      _prob->newConstraint(f, 0, 0, "2dimHull(BinVar)");
    }
    break;
  case Integer:
    //assert(LB!=-INFINITY && UB!=INFINITY);
    effLB = std::ceil(LB);
    effUB = std::floor(UB);
    // Assuming integrality of bounds
    if (effLB!=LB || effUB!=UB || effLB>effUB)
    {
      std::cout<<"MIQCP::add2dimCvxHull() Warning: Inconsistent bound(s)."<<std::endl;
    }
    else
    {
      // Finite bounds
      if (LB!=-INFINITY && UB!=INFINITY)
      {
        // Upper envelope, Linear equality: xsqr <= (L+U)x-L*U
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(x, -(effLB+effUB));
        lf->addTerm(xsqr, 1.0);
        f = (FunctionPtr) new Function(lf);
        _prob->newConstraint(f, -INFINITY, -effLB*effUB, "2dimHull-upp(IntVar)");

        // Lower envelope,
        for (Double T = effLB; T<effUB-(1E-5); T=T+1)
        {
          //Segment of lower envelope: xsqr >= (T+T+1)*x - T*(T+1)
          lf = (LinearFunctionPtr) new LinearFunction();
          lf->addTerm(x, -(T+T+1.0));
          lf->addTerm(xsqr, 1.0);
          f = (FunctionPtr) new Function(lf);
          _prob->newConstraint(f, -T*(T+1), INFINITY, "2dimHull-low(IntVar)");
        }
      }
      else
      {
        std::cerr<<"Unbounded integral variables exists!"<<std::endl;
        // Lower envelop, quadratic constraint x^2 - xsqr <= 0
        qf = (QuadraticFunctionPtr) new QuadraticFunction();
        qf->addTerm(x,x,1.0);
        lf = (LinearFunctionPtr) new LinearFunction();
        lf->addTerm(xsqr, -1.0);
        f = (FunctionPtr) new Function(lf, qf);
        _prob->newConstraint(f, -INFINITY, 0, "2dimHull-low(Cont)");
      }
    }
    break;

  case Continuous:
    assert(LB!=-INFINITY && UB!=INFINITY);
    if (LB!=-INFINITY && UB!=INFINITY)
    {
      // Upper envelope, Linear equality: x^2 <= (L+U)x-L*U
      lf = (LinearFunctionPtr) new LinearFunction();
      lf->addTerm(x, -(LB+UB));
      lf->addTerm(xsqr, 1.0);
      f = (FunctionPtr) new Function(lf);

      // bug-fix
      //_prob->newConstraint(f, -INFINITY, -effLB*effUB, "2dimHull-upp(Cont)");
      _prob->newConstraint(f, -INFINITY, -LB*UB, "2dimHull-upp(Cont)");
    }
    // Lower envelop, quadratic constraint x^2 - xsqr <= 0
    qf = (QuadraticFunctionPtr) new QuadraticFunction();
    qf->addTerm(x,x,1.0);
    lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(xsqr, -1.0);
    f = (FunctionPtr) new Function(lf, qf);
    _prob->newConstraint(f, -INFINITY, 0, "2dimHull-low(Cont)");
    break;

  default:
    break;
  }
}

void MIQCP::initRelax()
{
  Double mineig;

  QuadraticFunctionPtr newqf;
  LinearFunctionPtr newlf;
  FunctionPtr newf;
  Double newub;

  if (_type!=ProbTypeBoxQP && _type!=AMPLstubInput)
  {
    std::cout<<"Error: Function MIQCP initRelax() only support BoxQP at this moment."<<std::endl;
    return;
  }

  // Enumerate all variables, create a lifted pair if it exists in some nonconvex
  // quadratic constraint
  // First record all the variables to lift
  //VarVector toaddlift;
  VariableSet toaddlift;
  for (VariableConstIterator it = _prob->varsBegin(); it!=_prob->varsEnd();
      it++)
  {
    for (std::vector<QuadConsPtr>::const_iterator row=_ncvxRows.begin();
        row!=_ncvxRows.end(); row++)
    {
      if ((*row)->hasVarInQuad((*it)))
      {
        //toaddlift.push_back((*it));
        toaddlift.insert(*it);
        std::cout<<"Variable "<<(*it)->getName()<<" added for lifting"<<std::endl;
        break;
      }
    }
  }

  // Lift variables, record in varmap, and add 2-dim convex hull constraints
  //for (VariableConstIterator it = toaddlift.begin();
  //     it != toaddlift.end(); it++)
  for (VarSetConstIterator it = toaddlift.begin();
         it != toaddlift.end(); it++)
  {
    Double UB = ((*it)->getUb())*((*it)->getUb());
    ConstVariablePtr sqrvar = _prob->newVariable(0, UB, Continuous, ("sqr"+(*it)->getName()));
    _varmap->LinkRelaxSqr((*it), sqrvar);
    add2dimCvxHull((*it), sqrvar);
    std::cout<<"Variable "<<(*it)->getName()<<" lifted"<<std::endl;
  }

  std::cout<<"(MIQCP::initRelax) " << toaddlift.size() << " lifting variables added"
      <<std::endl;

  // Establish initial relaxation for each nonconvex quadratic constraints
  for (std::vector<QuadConsPtr>::const_iterator row=_ncvxRows.begin();
      row!=_ncvxRows.end(); row++)
  {
    mineig = (*row)->getMinEig();
    std::cout<<"Nonconvex Cons: mineig="<<mineig<<std::endl;

    if (mineig<0)
    {
      std::pair<FunctionPtr, Double> RelaxPair =
          (*row)->genCvxQuad(InitialRelax, _sol, _varmap);
      newf = RelaxPair.first;
      newub = RelaxPair.second;
      _prob->newConstraint(newf,-INFINITY,newub);
      std::cout<<"Relaxed constraint added."<<std::endl;
    }
  }

  std::cout<<"(MIQCP::initRelax) " << _ncvxRows.size()
      << " nonconvex quadratic constraints relaxed" <<std::endl;
}

// Solve the relaxation stored in the engine, set the _engstatus and _sol
// [out] _engstatus;
// [out] _sol;
// [out] comptime;
void MIQCP::solveEngRelax(Double & comptime, bool useInitial)
{
  if (useInitial)
  {
    _prob->setInitialPoint(_sol->getPrimal());
  }

  Timer* ptimer = _env->getNewTimer();

  ptimer->start();

  _engstatus = _engine->solve();
  if (ProvenOptimal==_engstatus || ProvenLocalOptimal==_engstatus)
  {
    _sol = _engine->getSolution();
  }
  else
  {
	std::cout<<"Warning: Engine status = "<<_engstatus<<std::endl;
  }

  comptime = ptimer->query();

  ptimer->stop();
  delete ptimer;
}

void MIQCP::writeSol(bool ifwriteprimal)
{
  Int err;
  Double objval = _prob->getObjValue(_sol->getPrimal(),&err);
  if (_engstatus == ProvenOptimal || _engstatus == ProvenLocalOptimal) {
    std::cout << _me << " obj="
        <<std::setw(8)<<std::setprecision(5) << objval << std::endl;
    if (ifwriteprimal)
    {
      _sol->writePrimal(std::cout);
    }
  } else {
    std::cout << _me << "engine has a problem. status = " << _engstatus << std::endl;
  }
}

bool MIQCP::separateByDiagPerturbation(Double& septime)
{
  if (PRINTLEVEL)
  {
	std::cout<<"Entering MIQCP::separateByDiagPerturbation"<<std::endl;
  }
  std::vector<QuadConsPtr>::const_iterator ncvxrow;
  std::pair<FunctionPtr, Double> NewRelaxPair;
  FunctionPtr f;
  Double UB;

  bool CutFound = false;
  Timer* ptimer = _env->getNewTimer();
  septime = 0.0;

  // Iterate over all nonconvex quadratic functions
  for (ncvxrow = _ncvxRows.begin(); ncvxrow != _ncvxRows.end(); ncvxrow++)
  {
    ptimer->start();
    NewRelaxPair = (*ncvxrow)->genCvxQuad(Perturbe, _sol, _varmap);
    septime += ptimer->query();
    ptimer->stop();

    //std::cout<<"Separation subroutine took "<<ptimer->query()<<" sec."<<std::endl;
    //delete ptimer;

    f = NewRelaxPair.first;
    UB = NewRelaxPair.second;
    int err = 0;

    Double fval =  f->eval(_sol->getPrimal(),&err);
    if (PRINTLEVEL)
    {
      std::cout<<"Separation result: Function evaluated at "<<fval
          <<" while UB = "<<UB<<std::endl;
    }

    if (fval>UB+1E-8)
    {
      //std::cout<<"Added f:"<<std::endl;
      //f->write(std::cout);
      ConstraintPtr newcons = _prob->newConstraint(f,-INFINITY,UB);
      CutFound = true;
    }
    else
    {
      //std::cout<<"No additional constraint added."<<std::endl;
      CutFound = false;
    }
  }
  delete ptimer;
  return CutFound;
}
