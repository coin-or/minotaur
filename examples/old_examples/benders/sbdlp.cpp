// GTG ============================================================
// this program is used to solve large scale LP problem using the
// Benders Decomposition. In the first stage, it read in an LP, and
// solve the LP using some LP solver through the OSI (open solver
// interface)
// GTG ============================================================

#include <omp.h>
#include "problem.hpp"
//#include "timer.hpp"

using namespace std; 
using UUID::uuid;
using Type::INF;

int main (const int argc, char **argv)
{
  scoped_ptr<uuid>    uid(new uuid(1000));
  size_t nocv = atoi(argv[2]);	// number of common variables
  size_t nodv = atoi(argv[3]);	// number of decision variables
  
  agent<variable>     agVar;	// agent for all the variables
  agent<objective>    agObj;	// agent for all the objectives
  agent<constraint>   agCon;	// agent for all the constraints 
  
  variableHandler   allVh;	// handler for all the variables
  variableHandler   comVh(&agVar, uid, nocv, "x", 0, 10);
  variableHandler   decVh(&agVar, uid, nodv, "y", 0, 10);
  
  allVh.add(comVh);		
  allVh.add(decVh);
  
  shared_ptr<OsiSolverInterface> spSI(new OsiClpSolverInterface); 
  shared_ptr<OsiSolverInterface> mpSI(new OsiClpSolverInterface); 
  
  spSI->setObjSense(1); spSI->messageHandler()->setLogLevel(0); 
  mpSI->setObjSense(1); mpSI->messageHandler()->setLogLevel(0); 
  
  
  double lbl = -INF, lbg = -INF, ubl = INF, ubg = INF, cutShift; 
  
  cout.precision(2);
  cout << fixed << showpoint;
  
  // ... define the master probelm
  scoped_ptr<problem> masterP (new problem(uid->next(),"MAS",&decVh,mpSI,&agObj,&agCon));
  scoped_ptr<problem> subP    (new problem(uid->next(),"SUB",&allVh,spSI,&agObj,&agCon)); 
  
  // ... read in the sub problem 
  subP->read(argv[1], uid);
  
  // ... set an initial guess of the decision variables for the sub-problem
  for(size_t i=0;i<nodv;i++) decVh[i]->fix(0);
  subP->load();
  
  // ============================================================
  size_t iter = 0;  short direction = 0;
  while(ubg - lbg > 1.0E-3 && iter < 10){
    
    subP->save(iter);
    
    iter == 0 ? subP->initialSolve() : subP->resolve();
    
    objective*  ocut = NULL;
    constraint* icut = NULL;
    if(subP->status() == 1){	// sub-problem is optimal
      subP->show(cout,1, iter);
      ubl = spSI->getObjValue(); 
      
      if(ubl < ubg) ubg = ubl;
      
      if(ubg - lbg < 1.0E-3) {
	cout << left << setw(6) << "BOUND:" 
	     << right << setw(6) << lbg 
	     << right << setw(6) << lbl 
	     << right << setw(6) << ubl 
	     << right << setw(6) << ubg 
	     << endl;
	direction = 1;
	break;
      }
      
      // generate the optimum cut for the master problem
      linearfunc cutLf(subP->cut(1, &comVh, &decVh, cutShift));
      ocut  = masterP->newObj(uid, 1, cutLf, cutShift);
      if(iter > 0) masterP->addObj(ocut);
      
    } else if(subP->status() == 0){ // sub-problem is infeasible
      
      subP->show(cout, 0, iter);	    
      subP->solveAux();		// solve the  auxiliary problem for the row prices
      subP->show(cout, 0, iter);
      
      // generate the infeasible cut
      linearfunc cutLf(subP->cut(0, &comVh, &decVh,  cutShift));
      icut  = masterP->newCon(uid,cutLf, -INF, -cutShift);
      if(iter > 0) masterP->addCon(icut);
      
    } else{			// infeasible initial guess!
      cout << "sub problem error!" << endl;
    }
    // ... solve the master problem
    
    // ... free the decision variables
    for(size_t i=0;i<nodv;i++) decVh[i]->free(0,1);
    
    if(iter == 0){
      masterP->load();
      masterP->save(0);
      masterP->initialSolve();
    }else{
      masterP->save(iter);
      masterP->resolve();
    }
    
    if(masterP->status() == 1){	// master problem is feasible
      masterP->show(cout,1, iter); // 
      lbl = mpSI->getObjValue();
      if(lbl > lbg) lbg = lbl;
      if(ubg - lbg < 1.0E-3) {
	cout << left << setw(6) << "BOUND:" 
	     << right << setw(6) << lbg 
	     << right << setw(6) << lbl 
	     << right << setw(6) << ubl 
	     << right << setw(6) << ubg 
	     << endl;
	direction = -1;
	break;
      }
    }else{
      cout << "master problem error!" << endl;
    }
    
    // print the bounds information
    cout << left << setw(6) << "BOUND:" 
	 << right << setw(6) << lbg 
	 << right << setw(6) << lbl 
	 << right << setw(6) << ubl 
	 << right << setw(6) << ubg 
	 << endl;
    
    // prepare for the next iteration
    for(size_t i=0;i<nodv;i++) {
      decVh[i]->fix();
      subP->updateVarFx(nocv+i); 
    }
    
    iter++;
  }
  
  // print the final solution
  // ========================
  if(direction == -1){
    for(size_t i=0;i<nodv;i++) {
      decVh[i]->fix();
      subP->updateVarFx(nocv+i); 
    }
    //... solve the sub-problem
    subP->resolve();
    subP->show(cout,1,9999);
  }else if(direction == 1){
    subP->show(cout,1,9999);
  }else{
    cout << "problem has not converged yet!" << endl;
  }
  
}
