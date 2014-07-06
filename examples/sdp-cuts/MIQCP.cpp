/*
 * MIQCP.cpp
 *
 *  Created on: Jun 26, 2014
 *      Author: hongbodong
 */

#include "MIQCP.h"
#include "CDlogdet.h"
//#include <boost/lexical_cast.hpp>

using namespace Minotaur;

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

  //_eigs->write(std::cout);

  // Set the min and max eigenvalue, and _HessScale
  _mineig = INFINITY;
  _maxeig = -INFINITY;
  for (EigenPairConstIterator eigp = _eigs->begin(); eigp != _eigs->end(); eigp++)
  {
    _mineig = fmin(_mineig, (*eigp).first);
    _maxeig = fmax(_maxeig, (*eigp).first);
  }
  _HessScale = fmax(fabs(_mineig), fabs(_maxeig));

  // Now we start to fill _HessPtr
  _HessScaledPtr = new Double[nvars*nvars];
  for (UInt i=0; i<nvars*nvars; i++)
  {
    _HessScaledPtr[i] = 0.0;
  }

  UInt seq = 1;
  for (VarIntMapConstIterator varintpair=qf->varsBegin();
      varintpair != qf->varsEnd(); varintpair++, seq++)
  {
    _order.insert(std::pair<ConstVariablePtr, UInt>((*varintpair).first, seq));
  }

//  std::cout<<"variable order:"<<std::endl;
//  for (VarIntMapConstIterator varintpair=_order.begin();
//      varintpair != _order.end(); varintpair++)
//  {
//    std::cout<<((*varintpair).first)->getName()<<" "<<((*varintpair).second)<<std::endl;
//  }

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
      _HessScaledPtr[nvars*(idx1-1)+idx2-1] = weight/_HessScale;
    else
    {
      _HessScaledPtr[nvars*(idx1-1)+idx2-1] = weight/(2*_HessScale);
      _HessScaledPtr[nvars*(idx2-1)+idx1-1] = weight/(2*_HessScale);
    }
  }

  //std::cout<<"Construction of QuadCons"<<std::endl;
  //printMat(_HessScaledPtr, nvars);
//  for (UInt i=1; i<=nvars; i++)
//  {
//    for (UInt j=1; j<=nvars; j++)
//    {
//      std::cout<<std::setw(5)<<std::setprecision(3)<<_HessPtr[nvars*(j-1)+i-1];
//    }
//    std::cout<<std::endl;
//  }
}

QuadCons::~QuadCons()
{
  delete _HessScaledPtr;
}

std::pair<FunctionPtr, Double> QuadCons::genCvxQuad(ConstSolutionPtr sol, CutMethod mtd)
{
  //std::cout<<"Entering QuadCons::genCvxQuad"<<std::endl;

  FunctionPtr f;
  Double UB;
  QuadraticFunctionPtr qf = _f->getQuadraticFunction();
  LinearFunctionPtr lf = _f->getLinearFunction();
  UInt n = qf->getNumVars();
  const Double* psol = sol->getPrimal();

  boost::scoped_array<Double> wneg(new Double[n]);
  boost::scoped_array<Double> wpos(new Double[n]);
  boost::scoped_array<Double> d(new Double[n]);

  if (mtd==Perturbe)
  {
    Double perturbfactor = 0.0;
    Double maxwpos = 0.0;
    for (VarIntMapConstIterator varintpair=qf->varsBegin();
        varintpair != qf->varsEnd(); varintpair++)
    {
      // Variable Pointer
      ConstVariablePtr var = varintpair->first;

      // Index in _HessScaledPtr
      UInt indx = (_order.find(var))->second;

      indx -= 1;

      // Solution value
      Double dx = psol[var->getIndex()];

      // Set wneg and wpos
      wneg[indx] = QuadCons::evalLenv((ConstVariablePtr) var, dx) - dx*dx;
      wpos[indx] = QuadCons::evalUenv((ConstVariablePtr) var, dx) - dx*dx;

      // Set initial d
      d[indx] = 1.5*std::abs(std::min(0.0, _mineig))/_HessScale;

//      std::cout<<var->getName()<<"="<<std::setw(6)<<std::setprecision(2)<<dx<<" at indx:"<<indx;
//      std::cout<<"  wneg:"<<std::setw(6)<<std::setprecision(2)<<wneg[indx];
//      std::cout<<"  wpos:"<<std::setw(6)<<std::setprecision(2)<<wpos[indx];
//      std::cout<<"  d(init):"<<std::setw(6)<<std::setprecision(2)<<d[indx]<<std::endl;

      perturbfactor += wpos[indx];
      maxwpos = fmax(wpos[indx],maxwpos);
    }
    perturbfactor /= (n);

    //Adjust wpos
    //std::cout<<"Adjustment: perturbfactor = "<<perturbfactor<<std::endl;
    for (UInt i=0; i<n; i++)
    {
      wneg[i] /= maxwpos;
      wpos[i] /= maxwpos;
      //wneg[i] += perturbfactor/maxwpos;
      wpos[i] += perturbfactor/maxwpos;

//      std::cout<<std::setw(6)<<std::setprecision(2)<<" at indx:"<<i;
//      std::cout<<"  wneg:"<<std::setw(6)<<std::setprecision(2)<<wneg[i];
//      std::cout<<"  wpos:"<<std::setw(6)<<std::setprecision(2)<<wpos[i];
//      std::cout<<"  d(init):"<<std::setw(6)<<std::setprecision(2)<<d[i]<<std::endl;

    }

    //Double sigma = 100;
    Double sigma = 1.0396;
    //Double sigma = 10.396;
    Int maxIter = 500*n;

    //datacheck(qf->getNumVars(), _HessScaledPtr, wneg.get(), wpos.get(), sigma, maxIter, d.get(), NULL, NULL, NULL);


    CDlogdet_nonsmooth(n, _HessScaledPtr, wneg.get(), wpos.get(), sigma, maxIter, d.get(), NULL, NULL, NULL);

    //datacheck(qf->getNumVars(), _HessPtr, wneg.get(), wpos.get(), sigma, maxIter, d.get(), NULL, NULL, NULL);

    //Print computed d
//    std::cout<<"computed d:"<<std::endl;
//    for (UInt i=0; i<n; i++)
//    {
//      //std::cout<<std::setw(8)<<std::setprecision(3)<<d[i]*_HessScale;
//      std::cout<<std::setw(8)<<std::setprecision(3)<<d[i];
//    }
//    std::cout<<std::endl;

    // Now construct the relaxed constraint f(x) <= UB
    //std::cout<<"Warning: current implementation is specialized for boxqp"<<std::endl;

    QuadraticFunctionPtr newqf = cloneQuadF();
    LinearFunctionPtr newlf=cloneLinF();
    UB = getRHS();

    for (VarIntMapConstIterator varintpair = newqf->varsBegin(); varintpair != newqf->varsEnd(); varintpair++)
    {
      ConstVariablePtr var = (varintpair)->first;

      // Index in _HessScaledPtr
      UInt idx = (_order.find(var))->second;
      idx--;
      Double d_scaled = d[idx]*_HessScale;
      newqf->incTerm(var, var, d_scaled);
      newlf->incTerm(var,-d_scaled*((var)->getLb()+(var)->getUb()));
      UB += -d_scaled * ((var)->getLb()) * ((var)->getUb());

//      std::cout<<"Ind:"<<idx<<" Diagonal term: Original "<<qf->getWeight(var,var)<<" New "
//          <<newqf->getWeight(var, var) <<std::endl;
//
//      std::cout<<"Linear term: Original "<<lf->getWeight(var)<<" New "
//          <<newlf->getWeight(var) <<std::endl;

    }

    f = (FunctionPtr) new Function(newlf, newqf);
    //_prob->newConstraint(newf,-INFINITY,newub);
    //std::cout<<"Relaxed constraint added."<<std::endl;
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
  LinearFunctionPtr lf = (_f->getLinearFunction())->clone();
  return lf;
}

MIQCP::MIQCP(MINLPType minlptype, std::string filename)
{
  _me = "sdp-cuts: ";
  /// Call corresponding initialization
  switch (minlptype)
  {
  case ProbTypeBoxQP:
    _type = ProbTypeBoxQP;
    init_boxqp(filename);
    break;
  default:
    std::cout<<"Warning: undefined MINLPType"<<std::endl;
    break;
  }
}

void MIQCP::init_boxqp(std::string fname)
{
  int err = 0;
  _env = (EnvPtr) new Environment();
  _prob = (ProblemPtr) new Problem();

  _env->startTimer(err);

  std::ifstream fs;
  UInt nvars = 0;
  Double val;
  FunctionPtr f;
  LinearFunctionPtr lf;
  QuadraticFunctionPtr qf;
  VariablePtr v;
  ConstSolutionPtr sol;

  fs.open(fname.c_str(), std::ios::in);

  // Define the 'v' variable
  v = _prob->newVariable(-INFINITY, INFINITY, Continuous, "v");

  fs >> nvars;
  //std::cout<<"Number of variables:"<<(nvars+1)<<std::endl;

  std::string varname;
  for (UInt i=0; i<nvars; ++i) {
    _prob->newVariable(0.0, 1.0, Continuous);
  }

  // Now load the objective: min v + c'*x
  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(v, 1.0);
  for (UInt i=1; i<=nvars; i++)
  {
    fs >> val;
    lf->addTerm(_prob->getVariable(i), -val);
  }

  f = (FunctionPtr) new Function(lf);
  _prob->newObjective(f, 0.0, Minimize);

  // Put the quadratic constraint v >= x'*Q*x into _ncvxRows

  qf = (QuadraticFunctionPtr) new QuadraticFunction();
  for (UInt i=1; i<=nvars; i++)
  {
    for (UInt j=1; j<=nvars; j++)
    {
      fs >> val;
      qf->incTerm(_prob->getVariable(i), _prob->getVariable(j), -0.5*val);
    }
  }

  lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(v, -1.0);

  f = (FunctionPtr) new Function(lf, qf);

  QuadConsPtr quadrow = (QuadConsPtr) new QuadCons(_prob, f, 0.0, "v>=xt*Q*x");
  _ncvxRows.push_back(quadrow);

  //_prob->write(std::cout);
  fs.close();

  // Initial Relaxation of all nonconvex quadratic constraints.
  initRelax();

  _prob->setNativeDer();
  //_prob->write(std::cout);
}

void MIQCP::initRelax()
{
  Double mineig;

  QuadraticFunctionPtr newqf;
  LinearFunctionPtr newlf;
  FunctionPtr newf;
  Double newub;

  if (_type!=ProbTypeBoxQP)
  {
    std::cout<<"Error: Function MIQCP initRelax() only support BoxQP at this moment."<<std::endl;
    return;
  }

  for (std::vector<QuadConsPtr>::const_iterator row=_ncvxRows.begin();
      row!=_ncvxRows.end(); row++)
  {
    mineig = (*row)->getMinEig();
    std::cout<<"Nonconvex Cons: mineig="<<mineig<<std::endl;

    if (mineig<0)
    {

      mineig = 1.1*mineig;

      newqf = (*row)->cloneQuadF();
      newlf = (*row)->cloneLinF();

      newub = (*row)->getRHS();
      for (VarIntMapConstIterator varintpair = newqf->varsBegin(); varintpair != newqf->varsEnd(); varintpair++)
      {
        ConstVariablePtr var = (varintpair)->first;
        newqf->incTerm(var, var, -mineig);
        newlf->incTerm(var,mineig*((var)->getLb()+(var)->getUb()));
        newub += mineig * ((var)->getLb()) * ((var)->getUb());
      }

      newf = (FunctionPtr) new Function(newlf, newqf);
      _prob->newConstraint(newf,-INFINITY,newub);
      std::cout<<"Relaxed constraint added."<<std::endl;
    }
  }
}

void MIQCP::solveRelax()
{
  //std::cout<<"MIQCP::solveRelax() called. "<<std::endl;
  EnginePtr fSQPsolver = (FilterSQPEnginePtr) new FilterSQPEngine(_env);
  //EnginePtr fSQPsolver = (IpoptEnginePtr) new IpoptEngine(_env);
  //EngineStatus status;

  Timer* ptimer = _env->getNewTimer();

  //ptimer->start();
  fSQPsolver->load(_prob);
  _engstatus = fSQPsolver->solve();
  _sol = fSQPsolver->getSolution();
  //ptimer->stop();

  //ptimer->
  int err = 0;
  //std::cout<<"Total Timer: "<<_env->getTime(err)<<" sec."<<std::endl;
  //delete ptimer;
}

void MIQCP::writeSol()
{
  Int err;
  Double objval = _prob->getObjValue(_sol->getPrimal(),&err);
  if (_engstatus == ProvenOptimal || _engstatus == ProvenLocalOptimal) {
    std::cout << _me << "Primal feasible solution found; obj="
        <<std::setw(8)<<std::setprecision(5) << objval << std::endl;
    //_sol->writePrimal(std::cout);
  } else {
    std::cout << _me << "engine has a problem. status = " << _engstatus << std::endl;
  }
}

bool MIQCP::separateByDiagPerturbation()
{
  //std::cout<<"Entering MIQCP::separateByDiagPerturbation"<<std::endl;
  std::vector<QuadConsPtr>::const_iterator ncvxrow;
  std::pair<FunctionPtr, Double> NewRelaxPair;
  FunctionPtr f;
  Double UB;

  bool CutFound = false;
  for (ncvxrow = _ncvxRows.begin(); ncvxrow != _ncvxRows.end(); ncvxrow++)
  {
    //Timer* ptimer = _env->getNewTimer();
    //ptimer->start();
    NewRelaxPair = (*ncvxrow)->genCvxQuad(_sol, Perturbe);
    //ptimer->stop();

    //std::cout<<"Separation subroutine took "<<ptimer->query()<<" sec."<<std::endl;
    //delete ptimer;

    f = NewRelaxPair.first;
    UB = NewRelaxPair.second;
    int err = 0;

    Double fval =  f->eval(_sol->getPrimal(),&err);
    //std::cout<<"Separation result: Function evaluated at "<<fval
    //    <<" while UB = "<<UB<<std::endl;

    if (fval>UB)
    {
      _prob->newConstraint(f,-INFINITY,UB);
      //std::cout<<"New relaxed constraint added."<<std::endl;
      CutFound = true;
    }
    else
    {
      //std::cout<<"No additional constraint added."<<std::endl;
    }
  }
  return CutFound;
}
