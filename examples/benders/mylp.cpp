// GTG ============================================================
// this program is used to solve an LP only, and print the results.
// GTG ============================================================

#include "problem.hpp"

using namespace std; 
using UUID::uuid;
using Type::INF;

int main (const int argc, char **argv)
{
  scoped_ptr<uuid>    uid(new uuid(1000));
  size_t  nov = atoi(argv[2]);	// number of common variables
  
  
  agent<variable>     agVar;	   // agent for all the variables
  agent<objective>    agObj;	   // agent for all the objectives
  agent<constraint>   agCon;	   // agent for all the constraints 
  
  variableHandler     allVh(&agVar, uid, nov, "x", 0, 10); // handler for all the variables
  
  shared_ptr<OsiSolverInterface> clp(new OsiClpSolverInterface); 
  clp->setObjSense(1);
  clp->messageHandler()->setLogLevel(0); 
  
  // ... define the sub problem
  scoped_ptr<problem> prob  (new problem(uid->next(),"SUB",&allVh, clp,&agObj, &agCon)); 
  
  // ... read in the sub problem 
  prob->read(argv[1], uid);
  
  // ... solve the sub problem
  prob->load();
  prob->save(0); 
  prob->initialSolve();
  
  // show the final status
  // =====================
  if(prob->status()==1){
    cout << "optimal" << endl;
    cout << prob->status();
    prob->show(cout, prob->status());
  }else{
    cout << "infeasible" << endl;
    prob->solveAux();
    cout << "====================" << endl;
    cout << prob->status();
    prob->save(8);
    prob->show(cout, prob->status());
  }
  
  // prob->check();
  
  
}
