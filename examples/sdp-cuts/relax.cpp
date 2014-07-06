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

#include "MIQCPData.h"
#include "MIQCP.h"
using namespace Minotaur;

//typedef std::vector<std::pair<FunctionPtr, Double> > QuadConsVec;
//typedef std::vector<std::pair<FunctionPtr, Double> >::const_iterator QuadConsVecIterator;

void boxqp_cuttingplane(std::string fname)
{
  int err = 0;

  MIQCP miqcp_prob(ProbTypeBoxQP, fname);
  //MIQCP miqcp_prob(ProbTypeBoxQP, "../data/boxqp/basic/spar020-100-1.in");
  //miqcp_prob.writeProblem();
  miqcp_prob.solveRelax();
  //miqcp_prob.writeSol();

  for (UInt iter=1; iter<=50; iter++)
  {
    //std::cout<<"========================"<<std::endl;
    std::cout<<"Iteration"<<std::setw(4)<<iter<<"  |  ";
    bool CutFound = miqcp_prob.separateByDiagPerturbation();
    //miqcp_prob.writeProblem();
    if (CutFound)
    {
      miqcp_prob.solveRelax();
      //miqcp_prob.writeSol();
      std::cout<<std::setw(8)<<std::setprecision(5)<<miqcp_prob.getObj(err)<<"  ";
    }
    else
    {
      std::cout<<"  (No more cuts found.)"<<std::endl;
      break;
    }

    std::cout<<std::setw(8)<<std::setprecision(5)<<miqcp_prob.getTime(err)<<" sec."<<std::endl;
    //std::cout<<"========================"<<std::endl;
  }
}

/*function... might want it in some class?*/
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

int main(int argc, char **argv)
{
  std::vector<std::string> files;
  getdir("../data/boxqp/basic/", files);
  getdir("../data/boxqp/extended/", files);
  getdir("../data/boxqp/extended2/", files);
  //boost::regex inst_file_pattern("spar*.in");
  //std::tr1::regex inst_file_pattern("spar*.in");
  for (std::vector<std::string>::const_iterator it = files.begin();
      it!= files.end(); it++)
  {
    //boost::regex_match(file, what, inst_file_pattern);
    size_t found = it->find("spar");
    // Found "spar" in filename
    if ((found)!=std::string::npos)
    {
      //std::cout<<(*it)<<" at " <<found<<std::endl;
      std::cout<<"Now work on instance "<<(*it)<<std::endl;
      boxqp_cuttingplane(*it);
      //break;
    }
  }

  //std::cout<<"Now work on "<<files.front()<<std::endl;
  //boxqp_cuttingplane(files.front());


  return 0;
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
