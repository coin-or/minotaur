//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2014 The MINOTAUR Team.
//

/**
 * \file relax.cpp
 * \brief Apply quadratic cuts to a relaxation of a non-convex QP
 * \author Hongbo Dong, Washington State University
 */

// Put config stuff (by hand) here
//#define MINOTAUR_RUSAGE

#include <cmath> // for INFINITY
#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>

//#include "MIQCPData.h"
#include "MIQCP.h"

#include "MinotaurConfig.h"
#include "Environment.h"
#include "FilterSQPEngine.h"
#include "Function.h"
#include "IpoptEngine.h"
//#include "IpoptEngineTnlp.h"
#include "Option.h"
#include "LinearFunction.h"
#include "Problem.h"
#include "QuadraticFunction.h"
#include "Solution.h"
#include "Timer.h"
#include "Constraint.h"
#include "HessianOfLag.h"
#include "Eigen.h"


using namespace Minotaur;
//using namespace MINOTAUR_AMPL;

int getdir (std::string dir, std::vector<std::string> &files);
void doBoxQPtest();
void boxqp_cuttingplane(std::string fname);
void AMPL_cuttingplane(std::string fname);

int main(int argc, char **argv)
{
  //doBoxQPtest();
  if (argc !=2)
  {
    std::cerr << "Usage: sdp-cuts <nlfile>" << std::endl;
    exit(1);
  }

  AMPL_cuttingplane(argv[1]);

  //MIQCP miqcp_prob = MIQCP(AMPLstubInput, argv[1]);
  //miqcp_prob.separateByDiagPerturbation();
  return 0;
}

//Cutting-plane methods for general MIQCQP in AMPL format
void AMPL_cuttingplane(std::string fname)
{
  //std::cout<<"Entering cuttingplane"<<std::endl;
  int err = 0;
  Double relaxtime = 0.0;
  Double septime = 0.0;

  //MIQCP miqcp_prob(ProbTypeBoxQP, fname);
  MIQCP miqcp_prob = MIQCP(AMPLstubInput, fname);
  //miqcp_prob.writeProblem();

  if (ProvenOptimal!=miqcp_prob.getEngStatus() &&
      ProvenLocalOptimal != miqcp_prob.getEngStatus())
  {
    std::cerr<<"AMPL_cuttingplane() Error: initial relaxation solve"
        << "failed." <<std::endl;
    return;
  }
  //miqcp_prob.solveEngRelax(relaxtime);

  std::cout<<"                  obj    relaxTime  sepTime  TotTimer"<<std::endl;
  //UInt maxIter = 1;
  UInt maxIter = 50;
  for (UInt iter=1; iter<= maxIter; iter++)
  {
    //std::cout<<"========================"<<std::endl;
    std::cout<<"Iteration"<<std::setw(4)<<iter<<"  |  ";
    bool CutFound = miqcp_prob.separateByDiagPerturbation(septime);
    //miqcp_prob.writeProblem();
    if (CutFound)
    {
      //miqcp_prob.solveRelax(relaxtime);
      miqcp_prob.solveEngRelax(relaxtime, true);
      //miqcp_prob.writeSol(true);
      std::cout<<std::setw(10)<<std::setprecision(8)<<miqcp_prob.getObj(err)<<"  "
          <<std::setw(6)<<std::setprecision(4)<<relaxtime<<" "
          <<std::setw(6)<<std::setprecision(4)<<septime<<" "
          <<std::setw(8)<<std::setprecision(5)<<miqcp_prob.getTime(err)<<" sec."
          <<std::endl;
    }
    else
    {
      std::cout<<"  (No more cuts found.)"<<std::endl;
      break;
    }
  }
  std::cout<<"Tot. Time: "<<std::setw(8)<<std::setprecision(5)<<miqcp_prob.getTime(err)<<" sec."<<std::endl;
}


void doBoxQPtest()
{
  std::vector<std::string> files;
  getdir("../data/boxqp/basic/", files);
  getdir("../data/boxqp/extended/", files);
  getdir("../data/boxqp/extended2/", files);
  for (std::vector<std::string>::const_iterator it = files.begin();
      it!= files.end(); it++)
  {
    size_t found = it->find("spar");
    // If found "spar" in filename
    if ((found)!=std::string::npos)
    {
      std::cout<<"Instance: "<<(*it)<<std::endl;
      boxqp_cuttingplane(*it);
      break;
    }
  }
}

/*Enumerate all files in a folder and put in a vector,
 *  might want it in some class?*/
int getdir (std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(dir+std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

//Cutting-plane methods for BoxQP problem in its special file format
void boxqp_cuttingplane(std::string fname)
{
  //std::cout<<"Entering cuttingplane"<<std::endl;
  int err = 0;
  Double relaxtime = 0.0;
  Double septime = 0.0;

  MIQCP miqcp_prob(ProbTypeBoxQP, fname);
  miqcp_prob.writeProblem();

  miqcp_prob.solveEngRelax(relaxtime);
  miqcp_prob.writeSol(true);

  std::cout<<"        obj    relaxtime   TotTimer"<<std::endl;
  UInt maxIter = 50;
  for (UInt iter=1; iter<= maxIter; iter++)
  {
    //std::cout<<"========================"<<std::endl;
    std::cout<<"Iteration"<<std::setw(4)<<iter<<"  |  ";
    bool CutFound = miqcp_prob.separateByDiagPerturbation(septime);
    if (CutFound)
    {
      //miqcp_prob.writeProblem();
      //miqcp_prob.solveRelax(relaxtime);
      //miqcp_prob.writeProblem(false, true);
      miqcp_prob.solveEngRelax(relaxtime);
      //miqcp_prob.writeSol(true);
      std::cout<<std::setw(10)<<std::setprecision(8)<<miqcp_prob.getObj(err)<<"  "
          <<std::setw(6)<<std::setprecision(4)<<relaxtime<<" "
          <<std::setw(6)<<std::setprecision(4)<<septime<<" "
          <<std::setw(8)<<std::setprecision(5)<<miqcp_prob.getTime(err)<<" sec."
          <<std::endl;
    }
    else
    {
      std::cout<<"  (No more cuts found.)"<<std::endl;
      break;
    }
  }
  std::cout<<"Tot. Time: "<<std::setw(8)<<std::setprecision(5)<<miqcp_prob.getTime(err)<<" sec."<<std::endl;
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
