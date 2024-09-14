// GTG ============================================================
// this program is used to solve large scale LP problem using the
// Benders Decomposition. In the first stage, it read in an LP, and
// solve the LP using some LP solver through the OSI (open solver
// interface)
//
// this is a parallel version, the master node is used to solve the
// master problem, and all the other slave nodes are used to solve
// the sub-problems.
// GTG ============================================================

#include <boost/mpi.hpp>
#include <boost/timer.hpp>
#include <omp.h>
#include "benders.hpp"

using namespace std; 
namespace mpi = boost::mpi;
using boost::timer;

void show_bounds(const double lbg, const double lbl, 
		 const double ubl, const double ubg);

int main (int argc, char **argv)
{
  mpi::environment  env(argc, argv);
  mpi::communicator world;
  
  timer  tm, t0;
  size_t nocv = atoi(argv[2]);	// number of common variables
  size_t nodv = atoi(argv[3]);	// number of decision variables
  
  scoped_ptr<uuid>  uid(new uuid(1000));
  
  agent<variable>   agVar;	// agent for all the variables
  agent<objective>  agObj;	// agent for all the objectives
  agent<constraint> agCon;	// agent for all the constraints 
  
  variableHandler   allVh;	// handler for all the variables
  variableHandler   comVh(&agVar, uid, nocv, "x", 0, 10);
  variableHandler   decVh(&agVar, uid, nodv, "y", 0, 10);
  
  allVh.add(comVh);		
  allVh.add(decVh);
  
  shared_ptr<OsiSolverInterface> spSI(new OsiClpSolverInterface); 
  shared_ptr<OsiSolverInterface> mpSI(new OsiClpSolverInterface); 
  
  spSI->setObjSense(1); spSI->messageHandler()->setLogLevel(0); 
  mpSI->setObjSense(1); mpSI->messageHandler()->setLogLevel(0); 
  
  scoped_ptr<problem> subP   (new problem(uid->next(),"SUBP",&allVh,spSI,&agObj,&agCon)); 
  scoped_ptr<problem> masterP(new problem(uid->next(),"MASP",&decVh,mpSI,&agObj,&agCon));
  
  short  cutType;
  double lbl= -INF, lbg= -INF, ubl = INF, ubg = INF, cutShift; 
  vector<double> dvv(nodv), cutVal;
  vector<size_t> cutIdx;
  
  cout.precision(2);
  cout << fixed << showpoint;
  
  if(world.rank() == 0){	// generate the master problem on master processor
    fill(dvv.begin(), dvv.end(), 0);
    cout << "TIMER@" << setw(4) << world.rank() << ":"  
	 << setw(8) << tm.elapsed() << "; # " 
	 << "init p" << endl;
  }else{			// generate the sub-problem on slave processor
    subP->read(argv[1], uid);
    subP->load();
    
    cout << "TIMER@" << setw(4) << world.rank() << ":"  
	 << setw(8) << tm.elapsed() << "; # " 
	 << "init p" << endl;
  }
  
  size_t iter = 0;  short direction = 0;
  bool   stop = false;
  while(true){
    broadcast(world, dvv, 0);
    
    if(world.rank() != 0){
      timer t1;
      
      cutType = subP->sproc(iter, &comVh, &decVh, dvv, ubl, cutShift, cutIdx, cutVal);
      cout << "TIMER@" << setw(4) << world.rank() << ":"  
	   << setw(8) << t1.elapsed() << "; # " 
	   << "solve subp" << endl;
      
      t1.restart();
      mpi::request reqs[5];
      reqs[0] = world.isend(0, 0, cutType);
      reqs[1] = world.isend(0, 1, ubl);
      reqs[2] = world.isend(0, 2, cutShift);
      reqs[3] = world.isend(0, 3, cutIdx);
      reqs[4] = world.isend(0, 4, cutVal);
      mpi::wait_all(reqs, reqs+5);
      
      cout << "TIMER@" << setw(4) << world.rank() << ":"  
	   << setw(8) << t1.elapsed() << "; # " 
	   << "send subp data" << endl;
      
      t1.restart();
      mpi::request todo = world.irecv(0, 0, stop); todo.wait();

      cout << "TIMER@" << setw(4) << world.rank() << ":"  
	   << setw(8) << t1.elapsed() << "; # " 
	   << "waiting decision" << endl;
      
      if(stop) break;
    } else{			// master processor
      timer t1;
      
      
      mpi::request reqs[5];
      reqs[0] = world.irecv(1, 0, cutType);
      reqs[1] = world.irecv(1, 1, ubl);
      reqs[2] = world.irecv(1, 2, cutShift);
      reqs[3] = world.irecv(1, 3, cutIdx);
      reqs[4] = world.irecv(1, 4, cutVal);
      
      mpi::request todo;
      
      reqs[1].wait();
      
      cout << "TIMER@" << setw(4) << world.rank() << ":"  
	   << setw(8) << t1.elapsed() << "; # " 
	   << "waiting sub bound" << endl;
      
      if(ubl < ubg) ubg = ubl;
      if(ubg - lbg < 1.0E-3) {
	stop = true;
	todo = world.isend(1, 0, stop);
      	direction = 1;
      	show_bounds(lbg,lbl, ubl,ubg);
	todo.wait();
      	break;
      }
      
      mpi::wait_all(reqs, reqs+5);
      
      cout << "TIMER@" << setw(4) << world.rank() << ":" 
	   << setw(8) << t1.elapsed() << "; # " 
	   << "waiting all sub data" << endl;
      
      t1.restart();
      masterP->mproc(iter, uid, cutType, dvv, lbl, cutShift, cutIdx, cutVal);
      
      cout << "TIMER@" << setw(4) << world.rank() << ":" 
	   << setw(8) << t1.elapsed() << "; # " 
	   << "solve master p" << endl;
      
      if(lbl > lbg) lbg = lbl;
      if(ubg - lbg < 1.0E-3) {
	stop = true;
	todo = world.isend(1, 0, stop);
      	direction = -1;
      	show_bounds(lbg,lbl, ubl,ubg);
	todo.wait();
      	break;
      }
      
      iter < 10 ?  stop = false : stop = true;
      todo = world.isend(1, 0, stop);
      show_bounds(lbg,lbl, ubl,ubg);
      todo.wait();
    }
    ++iter;
  }
  
  broadcast(world, direction, 0);
  broadcast(world, dvv, 0);
  
  // print the final solution
  // ========================
  if(world.rank() == 1){
    subP->sproc(9999, &comVh, &decVh, dvv, ubl, cutShift, cutIdx, cutVal);
    if(direction != 0){
      subP->show(cout, 1, 9999);
    }else{
      cout << "problem has not converged yet!" << endl;
    }
  }else{
    cout << "TOTAL TIME ELAPSED: " << t0.elapsed() << " s" << endl;
  }
}

void show_bounds(const double lbg, const double lbl, 
		 const double ubl, const double ubg)
{
  cout << left << setw(6) << "BOUND:" 
       << right << setw(6) << lbg 
       << right << setw(6) << lbl 
       << right << setw(6) << ubl 
       << right << setw(6) << ubg 
       << endl;
}
