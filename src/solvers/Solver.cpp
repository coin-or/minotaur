//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2024 The Minotaur Team.
//

/**
 * \file Solver.cpp
 * \brief Define base class Solver.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <iomanip>

#include "MinotaurConfig.h"
#include "Logger.h"
#include "Option.h"
#include "Problem.h"
#include "Reader.h"
#include "Solver.h"
#include "Solution.h"
#include "Timer.h"

using namespace Minotaur;
const std::string Solver::me_ = "Solver: ";

Solver::Solver()
: env_(0),
  iface_(0),
  ownIface_(true)
{
}


Solver::~Solver()
{
  if (iface_ && ownIface_) {
    delete iface_;
  }
}


FileType Solver::getFileType(std::string fname)
{
  FileType ft = FileTypeNone;
  size_t pos = fname.find_last_of('.');

  if (pos != std::string::npos) {
    std::string ext = fname.substr(pos);
    for (size_t i=0; i<ext.length(); ++i) {
      ext[i] = std::toupper(ext[i]);
    }
    if (ext==".MPS") {
      ft = MPS;
    } else if (ext == ".NL") {
      ft = NL;
    } 
  } 
  return ft;
}


ProblemPtr Solver::readProblem(std::string fname, std::string dname,
                               std::string sname, int &err)
{
  FileType ft;
  OptionDBPtr options = env_->getOptions();
  double tstrt; 
  ProblemPtr p = 0;
  Reader rdr(env_);

  err = 0;
  tstrt = env_->getTime();
  ft = getFileType(fname);

  if (ft==MPS) {
    options->findString("interface_type")->setValue("mps");
    p = rdr.readMps(fname, err);
    if (err) {
      return 0;
    }
  } else if ((ft==NL) || 
             options->findFlag("ampl")->getValue()==1 ||
             options->findString("interface_type")->getValue()=="ampl") {
    iface_ = new MINOTAUR_AMPL::AMPLInterface(env_, sname);
    options->findString("interface_type")->setValue("ampl");
    p = iface_->readInstance(fname);
    env_->getLogger()->msgStream(LogInfo) << me_ 
      << "time used in reading instance = " << std::fixed 
      << std::setprecision(2) << env_->getTime()-tstrt << std::endl;
  } else {
    env_->getLogger()->errStream() << me_
      << "Unable to read the problem from file " << fname 
      << " Either provide a file with .nl or .mps extension or use -AMPL flag"
      << std::endl;
    err = 1;
    return 0;
  }

  if (!dname.empty()) {
    err = rdr.readSol(p, dname);
  }
  return p;
}


void Solver::setIface(MINOTAUR_AMPL::AMPLInterface* iface)
{
  iface_ = iface;
  ownIface_ = false;
}


int Solver::writeSol_(EnvPtr env, VarVector *orig_v, PresolverPtr pres,
                      SolutionPtr sol, SolveStatus status,
                      MINOTAUR_AMPL::AMPLInterface* iface)
{
  Solution* final_sol = 0;
  int err = 0;

  if (sol) {
    final_sol = pres->getPostSol(sol);
  }

  if (env->getOptions()->findFlag("ampl")->getValue() ||
      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(final_sol, status);
  } 
  
  if (final_sol && 
      env->getOptions()->findBool("display_solution")->getValue()) {
    final_sol->writePrimal(env->getLogger()->msgStream(LogNone), orig_v);
  } 

  if (final_sol) {
    delete final_sol;
  }
  return err;
}



