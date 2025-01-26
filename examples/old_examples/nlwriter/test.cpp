//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file test.cpp
 * \brief write an nl file
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>
#include <iostream>

#include "Environment.h"
#include "MinotaurConfig.h"
#include "Logger.h"
#include "NlWriter.h"
#include "Option.h"
#include "Problem.h"
#include "Types.h"

#include "AMPLHessian.h"
#include "AMPLInterface.h"
#include "AMPLJacobian.h"

using namespace Minotaur;

int main(int argc, char** argv)
{
  EnvPtr env      = (EnvPtr) new Environment();
  int err         = 0;
  OptionDBPtr options;
  MINOTAUR_AMPL::AMPLInterface* iface = 0;
  ProblemPtr oinst;    // instance that needs to be solved.
  const std::string me("nlwriter: ");
  double obj_sense = 1.0;
  NlWriter *nlw = new NlWriter(env);

  //env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  // Important to setup AMPL Interface first as it adds several options.
  iface = new MINOTAUR_AMPL::AMPLInterface(env, "nlwriter");

  // Parse command line for options set by the user.
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->readOptions(argc, argv);
  options = env->getOptions();
  
  oinst = iface->readInstance(options->findString("problem_file")->getValue());
  oinst->setNativeDer();
  env->getLogger()->msgStream(LogInfo) << "test: writing to file foo.nl"
                                       << std::endl;
  nlw->write(oinst, "foo.nl");

CLEANUP:
  if (iface) {
    delete iface;
  }
  if (nlw) {
    delete nlw;
  }
  return 0;
}


