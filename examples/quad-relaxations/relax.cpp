//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
// 

/** 
// This is example driver code to create different relaxations of
// Quadratically Constrained Optimization Problems.  
*/

// #include <cstdlib>
// #include <iostream>
// 
// #include "AMPLInterface.h"
// #include "Eigen.h"
// #include "Problem.h"
// 

#include "MinotaurConfig.h"

#include "Environment.h"
#include "Eigen.h"
#include "LPRelaxation.h"
#include "NLPRelaxation.h"
#include "Presolver.h"
#include "Problem.h"
#include "Timer.h"

#include "FilterSQPEngine.h"
#include "MinotaurDeconfig.h"
#include "IpoptEngine.h"
#include "OsiLPEngine.h"
#undef F77_FUNC_

#include "MinotaurConfig.h"
#include "RelaxQuad.h"
#include "AMPLInterface.h"

//#include "MinotaurDeconfig.h"
//#include "IpoptEngine.h"
//#include "MinotaurDeconfig.h"
//#include "MinotaurConfig.h"
//#include "opcode.hd"

using namespace Minotaur;


void show_help()
{
  std::cout << "Usage:" << std::endl
            << "To show version: relax -v (or --show_version yes) " 
            << std::endl
            << "To show all options: relax -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: relax --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


void add_flags(Minotaur::OptionDBPtr options)
{
  Minotaur::FlagOptionPtr f_option; // some flags provided by user
  f_option = (Minotaur::FlagOptionPtr) new Minotaur::Option<Minotaur::Bool>
    ("AMPL", "-AMPL option tells us to write .sol file for ampl.", 
      true, false);
  options->insert(f_option, true);

  f_option = (Minotaur::FlagOptionPtr) new Minotaur::Option<Minotaur::Bool>
    ("v", "-v option tells us to write version information.", 
      true, false);
  options->insert(f_option, true);

  f_option = (Minotaur::FlagOptionPtr) new Minotaur::Option<Minotaur::Bool>
    ("=", "-= option tells us to write all known options.", 
      true, false);
  options->insert(f_option, true);

  f_option = (Minotaur::FlagOptionPtr) new Minotaur::Option<Minotaur::Bool>
    ("?", "-? option tells us to write help message.", 
      true, false);
  options->insert(f_option, true);
}


int main(int argc, char** argv)
{
  EnvPtr env   = (EnvPtr) new Environment();
  // interface to AMPL (NULL)
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface();  
  Timer *timer = env->getNewTimer();
  OptionDBPtr options;
  ProblemPtr inst, p;    // instance that needs to be solved
  LPEnginePtr lp_e;
  NLPEnginePtr nlp_e;
  LPRelaxationPtr lpr;
  NLPRelaxationPtr nlpr;
  RelaxQuad *rq;

  timer->Start();

  options = env->getOptions();
  add_flags(options);

  // parse options
  env->readOptions(argc, argv);

  options->findString("interface_type")->setValue("AMPL");

  if (options->findBool("show_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    goto CLEANUP;
  }

  if (options->findBool("show_help")->getValue() ||
      options->findFlag("?")->getValue()) {
    show_help();
    goto CLEANUP;
  }

  if (options->findBool("show_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    std::cout << "Minotaur version " << env->getVersion() << std::endl;
    goto CLEANUP;
  }

  if (options->findString("problem_file")->getValue()=="") {
    show_help();
    goto CLEANUP;
  }

  // load the problem.
  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) new MINOTAUR_AMPL::AMPLInterface();
  inst = iface->readInstance(options->findString("problem_file")->getValue());

  inst->calculateSize();
  if (inst->isLinear() || inst->isQuadratic()) {
    PresolverPtr presolver = (PresolverPtr) new Presolver(inst);
    presolver->standardize();
    inst->calculateSize(true);
  } else {
    std::cout << "Instance is not quadratic or linear\n" << std::endl;
    goto CLEANUP;
  }

  // create an LO engine.
  lp_e = (Minotaur::OsiLPEnginePtr) new Minotaur::OsiLPEngine(); 

  // create an NLO engine.
  nlp_e = Minotaur::NLPEnginePtr(); //NULL
  if (options->findString("nlp_engine")->getValue()=="Filter-SQP") {
    std::cout << "using filter-sqp for nonlinear problems\n";
    nlp_e = (Minotaur::FilterSQPEnginePtr) new Minotaur::FilterSQPEngine();
  } else if (options->findString("nlp_engine")->getValue()=="IPOPT") {
    std::cout << "using ipopt for nonlinear problems\n";
    nlp_e = (Minotaur::IpoptEnginePtr) new Minotaur::IpoptEngine();
  }

  rq = new RelaxQuad(inst, options);

  rq->createRelaxation(RelaxQuad::TERMBYTERM);
  p = rq->getRelaxation();
  p->setQuadraticJacobian();
  p->setQuadraticHessian();
  nlp_e->load(p);
  nlp_e->solve();
  std::cout << "solution value of Mc+Secant+Convex = " 
    << nlp_e->getSolutionValue() << std::endl;
  std::cout << "status of Mc+Secant+Convex = " << 
    nlp_e->getStatusString() << std::endl;

  rq->createRelaxation(RelaxQuad::TERMBYTERM_OA);
  p = rq->getRelaxation();
  //p->write(std::cout);
  rq->solveUsingOA(lp_e);
  rq->solveUsingOA_Cuts(lp_e);
  
  delete rq;

  std::cout << "time used = "      << timer->Query() << std::endl;
  goto CLEANUP;

CLEANUP:
  if (iface) {
    delete iface;
  }
  delete timer;

  return 0;
}
