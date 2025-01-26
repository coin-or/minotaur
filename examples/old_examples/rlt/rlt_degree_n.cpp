//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 This is example driver code to take a problem and RLTize it
*/

// Put config stuff (by hand) here.  Why?
#define MINOTAUR_RUSAGE

//#include "MinotaurConfig.h"

#include <AMPLInterface.h>
#undef filename
#include <MultilinearHandler.h>
#include <Problem.h>
#include <LPRelaxation.h>
#include <OsiLPEngine.h>
#include <Function.h>
#include <vector>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <Environment.h>
#include <Constraint.h>
#include <Objective.h>
#include <iostream>
#include <LinearFunction.h>
#include <QuadraticFunction.h>
#include <PolynomialFunction.h>
#include <Variable.h>

using namespace Minotaur;
using namespace std;

void usage()
{
  std::cout << "usage: ./rlt_degree_n {name-of-.nl-file} {the degree of rlt}\n";
}

int
main(int argc, char** argv)
{
    if (argc < 3) {
    usage();
    return -1;
  }

  Minotaur::EnvPtr env1 = (Minotaur::EnvPtr) new Minotaur::Environment();
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface(env1);
  ProblemPtr inst, newInst;
  inst = iface->readInstance(argv[1]);
  newInst = iface->readInstance(argv[1]);
  int rltDegree = atoi(argv[2]);

  std::vector<FunctionPtr> v;
  std::vector<FunctionPtr> temp_v;
  std::vector<double> v_up;
  std::vector<double> v_lo;
  std::vector<double> temp_v_up;
  std::vector<double> temp_v_lo;

  for(ConstraintConstIterator con_it = inst->consBegin(); con_it != inst->consEnd(); ++con_it) {
    FunctionType con_typ = (*con_it)->getFunctionType();
    if(con_typ == Linear || con_typ == Quadratic || con_typ == Bilinear || con_typ == Polynomial) {
      FunctionPtr cf = (*con_it)->getFunction();
      v.push_back(cf);
      v_up.push_back((*con_it)->getUb());
      v_lo.push_back((*con_it)->getLb());
    }
  }

  int round = 1;
  while(round < rltDegree) {
    // we use these two integers to make sure that for the first
    // round the same constraint doesn't get built twice
    int con_cntr = -1;
    int v_cntr = -1;

    for(ConstraintConstIterator con_it = inst->consBegin(); con_it != inst->consEnd(); ++con_it) {
      con_cntr++;
      v_cntr = -1;
      // if the constraint is =, then just skip multiplying it with another constraint
      if((*con_it)->getUb() - (*con_it)->getLb() > 1e-6) {
	for(std::vector<FunctionPtr>::iterator vr_it = v.begin(); vr_it != v.end(); ++vr_it) {
	  v_cntr++;
	  
	  if(v_up[v_cntr] - v_lo[v_cntr] > 1e-6) {
	    
	    // if it is the first round, two constraints should be multiplied against eachother only once
	    if((round == 1 && v_cntr >= con_cntr) || round > 1) {
	      FunctionPtr cf = (*con_it)->getFunction();
	      FunctionPtr vf = *vr_it;
	      
	      FunctionType cf_typ = cf->getType();
	      FunctionType vf_typ = vf->getType();
	      if((cf_typ == Linear || cf_typ == Quadratic || cf_typ == Polynomial) && 
		 (vf_typ == Linear || vf_typ == Quadratic || vf_typ == Polynomial)) {
		
		// Break down the constraint function
		LinearFunctionPtr lcf = cf->getLinearFunction();
		QuadraticFunctionPtr qcf = cf->getQuadraticFunction();
		NonlinearFunctionPtr ncf = cf->getNonlinearFunction();
		PolyFunPtr pcf;
		if(ncf)
		  pcf = boost::dynamic_pointer_cast <PolynomialFunction> (ncf);
		
		// Break down the v function
		LinearFunctionPtr lvf = vf->getLinearFunction();
		QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
		NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
		PolyFunPtr pvf;
		if(nvf)
		  pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
		
		// Multiply parts of the two functions against eachother
		QuadraticFunctionPtr q_lcf_lvf;
		PolyFunPtr p_lcf_qvf;
		PolyFunPtr p_lcf_pvf;
		PolyFunPtr p_qcf_lvf;
		PolyFunPtr p_qcf_qvf;
		PolyFunPtr p_qcf_pvf;
		PolyFunPtr p_pcf_lvf;
		PolyFunPtr p_pcf_qvf;
		PolyFunPtr p_pcf_pvf;
		
		// Multiply the linear part of cf to the linear part of vf
		if(lcf && lvf)
		  q_lcf_lvf = lcf * lvf;
		
		// Multiply the linear part of cf to the quadratic part of vf
		if(lcf && qvf)
		  p_lcf_qvf = qvf * lcf;
		
		// Multiply the linear part of cf to the polynomial part of vf
		if(lcf && nvf)
		  p_lcf_pvf = pvf * lcf;
		
		// Multiply the quadratic part of cf to the linear part of vf
		if(qcf && lvf)
		  p_qcf_lvf = qcf * lvf;
		
		// Multiply the quadratic part of cf to the quadratic part of vf
		if(qcf && qvf)
		  p_qcf_qvf = qcf * qvf;
		
		// Multiply the quadratic part of cf to the polynomial part of vf
		if(qcf && nvf)
		  p_qcf_pvf = pvf * qcf;
		
		// Multiply the polynomial part of cf to the linear part of vf
		if(ncf && lvf)
		  p_pcf_lvf = pcf * lvf;
		
		// Multiply the polynomial part of cf to the quadratic part of vf
		if(ncf && qvf)
		  p_pcf_qvf = pcf * qvf;
		
		// Multiply the polynomial part of cf to the polynomial part of vf
		if(ncf && nvf)
		  p_pcf_pvf = pcf * pvf;
		
		// Now add them up
		// - Polynomial part
		PolyFunPtr p_mult = p_lcf_qvf + p_lcf_pvf + p_qcf_lvf + p_qcf_qvf + 
		  p_qcf_pvf + p_pcf_lvf + p_pcf_qvf + p_pcf_pvf;
		
		NonlinearFunctionPtr n_mult = boost::dynamic_pointer_cast <NonlinearFunction> (p_mult);
		LinearFunctionPtr dummy;
		
		FunctionPtr mult = (FunctionPtr) new Function(dummy, q_lcf_lvf, n_mult);

		// If both the constraint and v have lower bound
		if((*con_it)->getLb() > -1e10 && v_lo[v_cntr] > -1e10) {
		  // Find c * cf
		  LinearFunctionPtr lcf_constant = -v_lo[v_cntr] * lcf;
		  QuadraticFunctionPtr qcf_constant = -v_lo[v_cntr] * qcf;
		  PolyFunPtr pcf_constant = -v_lo[v_cntr] * pcf;
		  
		  // Find d * vf
		  LinearFunctionPtr lvf_constant = -(*con_it)->getLb() * lvf;
		  QuadraticFunctionPtr qvf_constant = -(*con_it)->getLb() * qvf;
		  PolyFunPtr pvf_constant = -(*con_it)->getLb() * pvf;
		  
		  // Sum them up
		  PolyFunPtr p_rltf;
		  QuadraticFunctionPtr q_rltf;
		  LinearFunctionPtr l_rltf;
		  FunctionPtr rltf;
		  
		  if(p_mult || pcf_constant || pvf_constant) {
		    p_rltf = p_mult + pcf_constant + pvf_constant;
		    q_rltf = q_lcf_lvf + qvf_constant + qcf_constant;
		    l_rltf = lcf_constant + lvf_constant;
		    
		    NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
		  }
		  else {
		    q_rltf = q_lcf_lvf + qcf_constant + qvf_constant;
		    l_rltf = lcf_constant + lvf_constant;
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
		  }
		  		
		  temp_v.push_back(rltf);
		  temp_v_lo.push_back(-1 * (*con_it)->getLb() * v_lo[v_cntr]);
		  temp_v_up.push_back(INFINITY);
		}
		
		// If constraint has a lower bound and v has an upper bound
		if((*con_it)->getLb() > -1e10 && v_up[v_cntr] < 1e10) {
		  // Find c * cf
		  LinearFunctionPtr lcf_constant = v_up[v_cntr] * lcf;
		  QuadraticFunctionPtr qcf_constant = v_up[v_cntr] * qcf;
		  PolyFunPtr pcf_constant = v_up[v_cntr] * pcf;
		  
		  // Find d * vf
		  LinearFunctionPtr lvf_constant = (*con_it)->getLb() * lvf;
		  QuadraticFunctionPtr qvf_constant = (*con_it)->getLb() * qvf;
		  PolyFunPtr pvf_constant = (*con_it)->getLb() * pvf;
		  
		  // Sum them up
		  PolyFunPtr p_rltf;
		  QuadraticFunctionPtr q_rltf;
		  LinearFunctionPtr l_rltf;
		  FunctionPtr rltf;
		  
		  QuadraticFunctionPtr neg_q_lcf_lvf = -1 * q_lcf_lvf;
		  
		  if(p_mult || pcf_constant || pvf_constant) {
		    PolyFunPtr neg_p_mult = -1 * p_mult;
		    p_rltf = neg_p_mult + pcf_constant + pvf_constant;
		    q_rltf = neg_q_lcf_lvf + qvf_constant + qcf_constant;
		    l_rltf = lvf_constant + lcf_constant;
		    
		    NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
		  }
		  else {
		    q_rltf = neg_q_lcf_lvf + qcf_constant + qvf_constant;
		    l_rltf = lcf_constant + lvf_constant;
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
		  }
		  
		  temp_v.push_back(rltf);
		  temp_v_lo.push_back((*con_it)->getLb() * v_up[v_cntr]);
		  temp_v_up.push_back(INFINITY);
		}
		
		// If constraint has an upper bound and v has a lower bound
		if((*con_it)->getUb() < 1e10 && v_lo[v_cntr] > -1e10) {
		  // Find c * cf
		  LinearFunctionPtr lcf_constant = v_lo[v_cntr] * lcf;
		  QuadraticFunctionPtr qcf_constant = v_lo[v_cntr] * qcf;
		  PolyFunPtr pcf_constant = v_lo[v_cntr] * pcf;
		  
		  // Find d * vf
		  LinearFunctionPtr lvf_constant = (*con_it)->getUb() * lvf;
		  QuadraticFunctionPtr qvf_constant = (*con_it)->getUb() * qvf;
		  PolyFunPtr pvf_constant = (*con_it)->getUb() * pvf;
		  
		  // Sum them up
		  PolyFunPtr p_rltf;
		  QuadraticFunctionPtr q_rltf;
		  LinearFunctionPtr l_rltf;
		  FunctionPtr rltf;
		  
		  QuadraticFunctionPtr neg_q_lcf_lvf = -1 * q_lcf_lvf;
		  
		  if(p_mult || pcf_constant || pvf_constant) {
		    PolyFunPtr neg_p_mult = -1 * p_mult;
		    p_rltf = neg_p_mult + pcf_constant + pvf_constant;
		    q_rltf = neg_q_lcf_lvf + qvf_constant + qcf_constant;
		    l_rltf = lcf_constant + lvf_constant;

		    NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
		  }
		  else {
		    q_rltf = neg_q_lcf_lvf + qcf_constant + qvf_constant;
		    l_rltf = lcf_constant + lvf_constant;
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
		  }
		  
		  temp_v.push_back(rltf);
		  temp_v_lo.push_back((*con_it)->getUb() * v_lo[v_cntr]);
		  temp_v_up.push_back(INFINITY);
		}
		
		// If constraint has an upper bound and v has an upper bound
		if((*con_it)->getUb() < 1e10 && v_up[v_cntr] < 1e10) {
		  // Find c * cf
		  LinearFunctionPtr lcf_constant = -v_up[v_cntr] * lcf;
		  QuadraticFunctionPtr qcf_constant = -v_up[v_cntr] * qcf;
		  PolyFunPtr pcf_constant = -v_up[v_cntr] * pcf;
		  
		  // Find d * vf
		  LinearFunctionPtr lvf_constant = -(*con_it)->getUb() * lvf;
		  QuadraticFunctionPtr qvf_constant = -(*con_it)->getUb() * qvf;
		  PolyFunPtr pvf_constant = -(*con_it)->getUb() * pvf;
		  
		  // Sum them up
		  PolyFunPtr p_rltf;
		  QuadraticFunctionPtr q_rltf;
		  LinearFunctionPtr l_rltf;
		  FunctionPtr rltf;
		  if(p_mult || pcf_constant || pvf_constant) {
		    p_rltf = p_mult + pcf_constant + pvf_constant;
		    q_rltf = q_lcf_lvf + qvf_constant + qcf_constant;
		    l_rltf = lvf_constant + lcf_constant;
		    
		    NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
		  }
		  else {
		    q_rltf = q_lcf_lvf + qcf_constant + qvf_constant;
		    l_rltf = lcf_constant + lvf_constant;
		    rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
		  }
		  
		  temp_v.push_back(rltf);
		  temp_v_lo.push_back(-1 * (*con_it)->getUb() * v_up[v_cntr]);
		  temp_v_up.push_back(INFINITY);
		}
	      }
	    }
	  }
	}
      }
    }

    // multiply variable bound constraints against regular constraints
   
    for(VariableConstIterator var_it = inst->varsBegin(); var_it != inst->varsEnd(); ++var_it) {
      v_cntr = -1;

      // If the variable has a lower bound
      if((*var_it)->getLb() > -1e-10) {
	LinearFunctionPtr lbf = LinearFunctionPtr(new LinearFunction());
	lbf->addTerm(*var_it, 1);

	// Iterate through the functions in v
	for(std::vector<FunctionPtr>::iterator vr_it = v.begin(); vr_it != v.end(); ++vr_it) {
	  v_cntr++;
	  
	  // If v is = then skip it for now 
	  if(v_up[v_cntr] - v_lo[v_cntr] > 1e-6) {
	    
	    FunctionPtr vf = *vr_it;

	    // Break down the v function
	    LinearFunctionPtr lvf = vf->getLinearFunction();
	    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
	    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
	    PolyFunPtr pvf;
	    if(nvf)
	      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
	    
	    // Multiply parts of the two functions against eachother
	    QuadraticFunctionPtr q_lbf_lvf;
	    PolyFunPtr p_lbf_qvf;
	    PolyFunPtr p_lbf_pvf;
	    
	    // Multiply the linear part of lbf to the linear part of vf
	    if(lbf && lvf)
	      q_lbf_lvf = lbf * lvf;
	    
	    // Multiply the linear part of lbf to the quadratic part of vf
	    if(lbf && qvf)
	      p_lbf_qvf = qvf * lbf;
	    
	    // Multiply the linear part of lbf to the polynomial part of vf
	    if(lbf && pvf)
	      p_lbf_pvf = pvf * lbf;
	    
	    // Now add them up
	    // - Polynomial part
	    PolyFunPtr p_mult = p_lbf_qvf + p_lbf_pvf;
	    
	    // If v has a lower bound
	    if(v_lo[v_cntr] > -1e10) {
	      // Find c * lbf
	      LinearFunctionPtr lbf_constant = -v_lo[v_cntr] * lbf;
	      
	      // Find lb * vf
	      LinearFunctionPtr lvf_constant = -(*var_it)->getLb() * lvf;
	      QuadraticFunctionPtr qvf_constant = -(*var_it)->getLb() * qvf;
	      PolyFunPtr pvf_constant = -(*var_it)->getLb() * pvf;
	      
	      // Sum them up
	      PolyFunPtr p_rltf;
	      QuadraticFunctionPtr q_rltf;
	      LinearFunctionPtr l_rltf;
	      FunctionPtr rltf;
	      
	      if(p_mult || pvf_constant) {
		p_rltf = p_mult + pvf_constant;
		q_rltf = q_lbf_lvf + qvf_constant;
		l_rltf = lvf_constant + lbf_constant;
		
		NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
	      }
	      else {
		q_rltf = q_lbf_lvf + qvf_constant;
		l_rltf = lbf_constant + lvf_constant;
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
	      }
	      
	      temp_v.push_back(rltf);
	      temp_v_lo.push_back(-1 * (*var_it)->getLb() * v_lo[v_cntr]);
	      temp_v_up.push_back(INFINITY);
	    }
	    
	    // If v has a upper bound
	    if(v_up[v_cntr] < 1e10) {
	      // Find c * lbf
	      LinearFunctionPtr lbf_constant = v_up[v_cntr] * lbf;
	      
	      // Find lb * vf
	      LinearFunctionPtr lvf_constant = (*var_it)->getLb() * lvf;
	      QuadraticFunctionPtr qvf_constant = (*var_it)->getLb() * qvf;
	      PolyFunPtr pvf_constant = (*var_it)->getLb() * pvf;
	      
	      // Sum them up
	      PolyFunPtr p_rltf;
	      QuadraticFunctionPtr q_rltf;
	      LinearFunctionPtr l_rltf;
	      FunctionPtr rltf;
	      
	      QuadraticFunctionPtr neg_q_lbf_lvf = -1 * q_lbf_lvf;
	      
	      if(p_mult || pvf_constant) {
		PolyFunPtr neg_p_mult = -1 * p_mult;
		p_rltf = neg_p_mult + pvf_constant;
		q_rltf = neg_q_lbf_lvf + qvf_constant;
		l_rltf = lvf_constant + lbf_constant;
		
		NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
	      }
	      else {
		q_rltf = neg_q_lbf_lvf + qvf_constant;
		l_rltf = lbf_constant + lvf_constant;
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
	      }

	      temp_v.push_back(rltf);
	      temp_v_lo.push_back((*var_it)->getLb() * v_up[v_cntr]);
	      temp_v_up.push_back(INFINITY);	      
	    }
	  }

	  // If v is a = constraint just multiply it with the variable
	  else {
	    FunctionPtr vf = *vr_it;
	    
	    // Break down the v function
	    LinearFunctionPtr lvf = vf->getLinearFunction();
	    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
	    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
	    PolyFunPtr pvf;
	    if(nvf)
	      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);

	    // XXX check to see if pvf has a linear and quadratic part
	    // if so remove them
	    
	    // Multiply parts of the two functions against eachother
	    QuadraticFunctionPtr q_lbf_lvf;
	    PolyFunPtr p_lbf_qvf;
	    PolyFunPtr p_lbf_pvf;
	    
	    // Multiply the linear part of lbf to the linear part of vf
	    if(lbf && lvf)
	      q_lbf_lvf = lbf * lvf;
	    
	    // Multiply the linear part of lbf to the quadratic part of vf
	    if(lbf && qvf)
	      p_lbf_qvf = qvf * lbf;
	    
	    // Multiply the linear part of lbf to the polynomial part of vf
	    if(lbf && pvf)
	      p_lbf_pvf = pvf * lbf;
	    
	    // Now add them up
	    // - Polynomial part
	    PolyFunPtr p_mult = p_lbf_qvf + p_lbf_pvf;
	    
	    // Find c * lbf
	    LinearFunctionPtr lbf_constant = -v_lo[v_cntr] * lbf;
	      
	    // Sum them up
	    PolyFunPtr p_rltf;
	    QuadraticFunctionPtr q_rltf;
	    LinearFunctionPtr l_rltf;
	    FunctionPtr rltf;
	      
	    if(p_mult) {
	      p_rltf = p_mult;
	      q_rltf = q_lbf_lvf;
	      l_rltf = lbf_constant;
	      
	      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
	      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
	    }
	    else {
	      q_rltf = q_lbf_lvf;
	      l_rltf = lbf_constant;
	      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
	    }
  
	    temp_v.push_back(rltf);
	    temp_v_lo.push_back(0);
	    temp_v_up.push_back(0);
	  }
	}
      }

      // if the variable has an upper bound
      v_cntr = -1;
      if((*var_it)->getUb() < 1e10) {
	LinearFunctionPtr ubf = LinearFunctionPtr(new LinearFunction());
	ubf->addTerm(*var_it, -1);

	// Iterate through the functions in v
	for(std::vector<FunctionPtr>::iterator vr_it = v.begin(); vr_it != v.end(); ++vr_it) {
	  v_cntr++;
	  
	  // If v is = then skip it for now 
	  if(v_up[v_cntr] - v_lo[v_cntr] > 1e-6) {
	    FunctionPtr vf = *vr_it;

	    // Break down the v function
	    LinearFunctionPtr lvf = vf->getLinearFunction();
	    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
	    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
	    PolyFunPtr pvf;
	    if(nvf)
	      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
	    
	    // Multiply parts of the two functions against eachother
	    QuadraticFunctionPtr q_ubf_lvf;
	    PolyFunPtr p_ubf_qvf;
	    PolyFunPtr p_ubf_pvf;
	    
	    // Multiply the linear part of ubf to the linear part of vf
	    if(ubf && lvf)
	      q_ubf_lvf = ubf * lvf;
	    
	    // Multiply the linear part of ubf to the quadratic part of vf
	    if(ubf && qvf)
	      p_ubf_qvf = qvf * ubf;
	    
	    // Multiply the linear part of ubf to the polynomial part of vf
	    if(ubf && pvf)
	      p_ubf_pvf = pvf * ubf;
	    
	    // Now add them up
	    // - Polynomial part
	    PolyFunPtr p_mult = p_ubf_qvf + p_ubf_pvf;
	    
	    // If v has a lower bound
	    if(v_lo[v_cntr] > -1e10) {
	      // Find c * ubf
	      LinearFunctionPtr ubf_constant = -v_lo[v_cntr] * ubf;
	      
	      // Find ub * vf
	      LinearFunctionPtr lvf_constant = (*var_it)->getUb() * lvf;
	      QuadraticFunctionPtr qvf_constant = (*var_it)->getUb() * qvf;
	      PolyFunPtr pvf_constant = (*var_it)->getUb() * pvf;
	      
	      // Sum them up
	      PolyFunPtr p_rltf;
	      QuadraticFunctionPtr q_rltf;
	      LinearFunctionPtr l_rltf;
	      FunctionPtr rltf;
	      
	      if(p_mult || pvf_constant) {
		p_rltf = p_mult + pvf_constant;
		q_rltf = q_ubf_lvf + qvf_constant;
		l_rltf = lvf_constant + ubf_constant;
		
		NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
	      }
	      else {
		q_rltf = q_ubf_lvf + qvf_constant;
		l_rltf = ubf_constant + lvf_constant;
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
	      }
	      
	      temp_v.push_back(rltf);
	      temp_v_lo.push_back((*var_it)->getUb() * v_lo[v_cntr]);
	      temp_v_up.push_back(INFINITY);
	    }
	    
	    // If v has a upper bound
	    if(v_up[v_cntr] < 1e10) {
	      // Find c * ubf
	      LinearFunctionPtr ubf_constant = v_up[v_cntr] * ubf;
	      
	      // Find ub * vf
	      LinearFunctionPtr lvf_constant = -1 * (*var_it)->getUb() * lvf;
	      QuadraticFunctionPtr qvf_constant = -1 * (*var_it)->getUb() * qvf;
	      PolyFunPtr pvf_constant = -1 * (*var_it)->getUb() * pvf;
	      
	      // Sum them up
	      PolyFunPtr p_rltf;
	      QuadraticFunctionPtr q_rltf;
	      LinearFunctionPtr l_rltf;
	      FunctionPtr rltf;
	      
	      QuadraticFunctionPtr neg_q_ubf_lvf = -1 * q_ubf_lvf;
	      
	      if(p_mult || pvf_constant) {
		PolyFunPtr neg_p_mult = -1 * p_mult;
		p_rltf = neg_p_mult + pvf_constant;
		q_rltf = neg_q_ubf_lvf + qvf_constant;
		l_rltf = lvf_constant + ubf_constant;
		
		NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
	      }
	      else {
		q_rltf = neg_q_ubf_lvf + qvf_constant;
		l_rltf = ubf_constant + lvf_constant;
		rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
	      }
    
	      temp_v.push_back(rltf);
	      temp_v_lo.push_back(-1 * (*var_it)->getUb() * v_up[v_cntr]);
	      temp_v_up.push_back(INFINITY);
	    }
	  }

	  /*
	  // If v is a = constraint
	  else {
	    FunctionPtr vf = *vr_it;
	    
	    // Break down the v function
	    LinearFunctionPtr lvf = vf->getLinearFunction();
	    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
	    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
	    PolyFunPtr pvf;
	    if(nvf)
	      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
	    
	    // Multiply parts of the two functions against eachother
	    QuadraticFunctionPtr q_ubf_lvf;
	    PolyFunPtr p_ubf_qvf;
	    PolyFunPtr p_ubf_pvf;
	    
	    // Multiply the linear part of ubf to the linear part of vf
	    if(ubf && lvf)
	      q_ubf_lvf = ubf * lvf;
	    
	    // Multiply the linear part of ubf to the quadratic part of vf
	    if(ubf && qvf)
	      p_ubf_qvf = qvf * ubf;
	    
	    // Multiply the linear part of ubf to the polynomial part of vf
	    if(ubf && pvf)
	      p_ubf_pvf = pvf * ubf;
	    
	    // Now add them up
	    // - Polynomial part
	    PolyFunPtr p_mult = p_ubf_qvf + p_ubf_pvf;
	    
	    // Find c * ubf
	    LinearFunctionPtr ubf_constant = -v_lo[v_cntr] * ubf;
	      
	    // Sum them up
	    PolyFunPtr p_rltf;
	    QuadraticFunctionPtr q_rltf;
	    LinearFunctionPtr l_rltf;
	    FunctionPtr rltf;
	      
	    if(p_mult) {
	      p_rltf = p_mult;
	      q_rltf = q_ubf_lvf;
	      l_rltf = ubf_constant;
		
	      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
	      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
	    }
	    else {
	      q_rltf = q_ubf_lvf;
	      l_rltf = ubf_constant;
	      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
	    }
	    
	    temp_v.push_back(rltf);
	    temp_v_lo.push_back(0);
	    temp_v_up.push_back(0);
	  }
	  */
	}
      }
    }

    v.clear();
    v_up.clear();
    v_lo.clear();

    for(std::vector<FunctionPtr>::iterator temp_v_it = temp_v.begin(); temp_v_it != temp_v.end(); ++temp_v_it)
      v.push_back(*temp_v_it);
    
    for(int i = 0; i < temp_v_up.size(); i++) {
      v_up.push_back(temp_v_up[i]);
      v_lo.push_back(temp_v_lo[i]);
    }
    

    // Add the new RLT constraints to the problem
    for(int i = 0; i < v.size(); i++) {
      ConstraintPtr rltCon = newInst->newConstraint(v[i], v_lo[i], v_up[i]);
    }

    // clear the temp_v vectors
    temp_v.clear();
    temp_v_up.clear();
    temp_v_lo.clear();

    round++;
  }


  //*********************************************************
  //*********************************************************
  // Up to this point the problem is RLTized
  // Next is to build the relaxation for the problem
  //*********************************************************
  //*********************************************************
  newInst->write(std::cout);

  LPRelaxationPtr lpr = LPRelaxationPtr(new LPRelaxation());
  MultilinearHandlerPtr mhandler = MultilinearHandlerPtr(new MultilinearHandler(newInst));

  bool isFeasible;
  int nCons = newInst->getNumCons();
  int nVars = newInst->getNumVars();
  vector<bool> c_list(nCons);

  mhandler->relax(lpr, isFeasible, c_list);

  Minotaur::EnvPtr env = (Minotaur::EnvPtr) new Minotaur::Environment();
  OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine(env));
  int nv = lpr->getNumVars();
  
  lpE->load(lpr);
  lpE->solve();
  string status = lpE->getStatusString();

  //  lpE->writeLP("blah");

  cout << "After solve : ";
  cout << lpE->getStatusString();
  cout << " , " <<lpE->getStatus();
  cout << " , " << lpr->getNumVars();
  cout << " , " << lpr->getNumCons();
  cout << " , " << lpE->getSolutionValue() << endl;
  /*
  Const double *sol = lpE->getSolution();
  /*
  for(int i=0; i<nv; i++)
    cout << " , " << sol[i];
    //    cout <<"var["<<i<<"] = " << sol[i]<<endl;
  cout << endl;
  */
}


