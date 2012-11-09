// GTG ===========================================================================
// $Date$
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: problem.hpp 716 2010-04-29 16:42:32Z xzhen $
// GTG ===========================================================================
#ifndef PROBLEM_HH_
#define PROBLEM_HH_

// ==============================================
// problem class is a collection of constrains,
// including objective like constrains and normal
// constrains. 
//
// it also includes the I/O, and interface to OSI
// solver
// ==============================================
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "uuid.hpp"
#include "type.hpp"
#include "agent.hpp"
#include "genobj.hpp"
#include "variablehandler.hpp"
#include "objective.hpp"
#include "constraint.hpp"
#include "CoinPackedVector.hpp"
#include "CoinPackedMatrix.hpp"
#include "OsiClpSolverInterface.hpp"

using namespace std;
using boost::shared_ptr;
using boost::scoped_ptr;
using Type::Double;
using UUID::uuid;
using Type::INF;

class problem : public genobj{ // it is always minimization, LP
private:
  const variableHandler* const pvar_;	// vector of all the variables
  
  shared_ptr<OsiSolverInterface>  si_;	// solver interface
  
  vector<objective*>   objs_;	// vector of objective like constraints
  vector<constraint*>  cons_;	// vector of constaints
  
  agent<objective>*    paObj_;	// archive for the objective functions
  agent<constraint>*   paCon_;	// archive for the constraints
  
  size_t nov()const{return pvar_->size();}; // number of variables
  size_t noc()const{return cons_.size(); }; // number of constraints
  size_t noo()const{return objs_.size(); }; // number of objective
  
  // mainly get the dual values after the initialSolve or resolve
  void update();
public:
  explicit problem(const size_t uid, 
		   const char*  tag,
		   const variableHandler* const allVar,
		   shared_ptr<OsiSolverInterface> &si,
		   agent<objective>*  paObj,
		   agent<constraint>* paCon) 
    :genobj(0,"obj"), pvar_(allVar), si_(si), paObj_(paObj), paCon_(paCon)
  {id(uid); name(tag);};
  
  void attach(agent<objective>*  const agObj){paObj_ = agObj;};
  void attach(agent<constraint>* const agCon){paCon_ = agCon;};
  
  // add a new constraint
  void add(constraint* const pc){cons_.push_back(pc);}; 
  void add(objective*  const po){objs_.push_back(po);}; 
  
  vector<constraint*> read(const char* const fnin, scoped_ptr<uuid> &uid);
  // ===========================================================================
  // read in the problem from a file and save all the newly created constraints.
  // ===========================================================================
  // the prolbelm supposed to have one linear obnjective function, and
  // multiple constaints the auxiliary variables for each constaints will be
  // turn on/off according to the constaints type.
  
  objective*  newObj(scoped_ptr<uuid>     &uid, 
		     const short          &relation, 
		     const linearfunc     &lf, 
		     const Double         &shift=0.0);
  
  objective*  newObj(scoped_ptr<uuid>     &uid, 
		     const short          &relation,
		     const vector<size_t> &indx,
		     const vector<Double> &coef,
		     const Double         &shift=0.0);  
  
  constraint* newCon(scoped_ptr<uuid>     &uid, 
		     const linearfunc     &lf, 
		     const Double         &lb = -INF,
		     const Double         &ub =  INF);
  
  constraint* newCon(scoped_ptr<uuid>     &uid, 
		     const vector<size_t> &indx,
		     const vector<Double> &coef,
		     const Double         &lb = -INF,
		     const Double         &ub =  INF);
  
  // update the bounds for only one constraints
  void updateVarLb(const size_t i,const double val);
  void updateVarUb(const size_t i,const double val);
  void updateVarFx(const size_t i);
  void updateVarFx(const size_t i,const double val);
  
  // update the bounds for only one constraints
  void updateConLb(const size_t i,const double val);
  void updateConUb(const size_t i,const double val);
  
  // set or update the lower and upper bounds of all the constraints
  void updateConLb(const size_t n,const double* const val);
  void updateConUb(const size_t n,const double* const val);
  
  void updateConLb(const size_t n,const size_t* const loc,const double* const val);
  void updateConUb(const size_t n,const size_t* const loc,const double* const val);
  
  void load();
  
  void solveAux();    // convert the current problem into an auxiliary problem
  
  void initialSolve(){si_->initialSolve(); update();};
  void resolve()     {si_->resolve();      update();};
  
  short sproc(const size_t iter,
	      const variableHandler* const comVar, // common variables
	      const variableHandler* const decVar, // decision variables
	      const vector<double> &dvv, 
	      double &bound, double &shift, 
	      vector<size_t> &idx, vector<double> &val);
  
  short mproc(size_t &iter,
	      scoped_ptr<uuid> &uid, 
	      const short cutType,
	      vector<double> &dvv, 
	      double &bound, const double &shift, 
	      const vector<size_t> &idx, const vector<double> &val);
  
  linearfunc cut(const short cutType,
		 const variableHandler* const comPv,
		 const variableHandler* const decPv,
		 double& shift);
  
  short status()const;		// 1: optimal; 0: infeasible; -1: error
  
  void show(ostream &os, const short stat, const size_t tag = 0);
  void check();			// print out some information to check the gap
  void save(const size_t tag);
  void writeMPS(const size_t tag);
  void addCon(const constraint* const pcon);
  void addObj(const objective*  const pobj);
  
};

#endif 

