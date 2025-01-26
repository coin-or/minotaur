// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file RCHandler.h
 * \author Vinay Chourasiya, IIT Bombay
 * \brief Used for tightening lower and upper bound of variables using
 * reduced costs
 */

#ifndef MINOTAURRCHANDLER_H
#define MINOTAURRCHANDLER_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "Problem.h"
#include "Function.h"
#include "Types.h"

namespace Minotaur {


struct RCStats {
  size_t nlb;   /// Number of lower bound changed
  size_t nub;   /// Number of Upper Bound Changed             
  double time;
}; 

class CutManager;
class Timer;
class SimpleCutMan;
class Environment;

/**
 * \brief Handler for reduced cost strengthening 
 */
class RCHandler : public Handler {

  
public:
  /**
   * \brief Constructor
   * \param [in] env Minotaur environment
   */
  RCHandler(EnvPtr env);
  
  /// Destroy.
  ~RCHandler();
   
 /// Base class method
  Branches getBranches(BrCandPtr, DoubleVector &, RelaxationPtr,
                       SolutionPoolPtr)
  {return Branches();};

  // Base class method
  void getBranchingCandidates(RelaxationPtr, 
                              const DoubleVector &, ModVector &,
                              BrVarCandSet &, BrCandVector &, bool &) {};

  // Base class method
  ModificationPtr getBrMod(BrCandPtr, DoubleVector &, RelaxationPtr,
                           BranchDirection)
  { return ModificationPtr(); };

       
  // Base class method. 
  std::string getName() const;

  // Base class method. 
  bool isFeasible(ConstSolutionPtr, RelaxationPtr, bool &, double &) 
  { return true; };

  // Base class method
  SolveStatus presolve(PreModQ *, bool *, Solution **) 
  { return Finished; };

  // Base class method
  bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  // Base class method.
  void postsolveGetX(const double *, UInt, DoubleVector *) {};

  // Base class method 
  void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) {};

  // Base class method. calls relax_().
  void relaxInitInc(RelaxationPtr, SolutionPool *, bool *) {};

  // Base class method.
  void relaxNodeFull(NodePtr, RelaxationPtr, bool *) {};

  // Base class method.
  void relaxNodeInc(NodePtr, RelaxationPtr, bool *) {};

 
  // Base class method. Used for tightening upper and lower bound of variables.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, ModVector &p_mods,
                ModVector &q_mods, bool *sol_found, SeparationStatus *status);
 
  // Show statistics.
  void writeStats(std::ostream &) const;
  
private:
  /// previous best known objective value (for root-node based Reduced Costs)
  double lastBest_;

  /// Pointer to environment's logger
  LoggerPtr logger_;

  /// For logging
  static const std::string me_;

  /// Array of duals (reduced costs) of variables of the root relaxation
  double *rootDuals_;
  
  /// root node relaxtion objective value
  double rootObj_;
  
  /// Array of duals (reduced costs) of variables of the root relaxation
  double *rootX_;
  
  /// For statistics.
  RCStats *stats_;
  
  /// for time calculation  
  const Timer* timer_;

  // brief copy the root node dual information 
  // \param[in] solution pointer
  // \param[in] relaxation problem pointer  
  void copyRootDetails_(ConstSolutionPtr sol,  RelaxationPtr rel); 

  // brief reduced cost fixing using dual information of node
  // \param[in] solution pointer
  // \param[in] relaxation problem pointer
  // \param[in] 
  // \param[in] incumbent solution value 
  // \param[in] relaxation object value
  // \param[in] x[v->getIndex] value
  // \param[in] reduced cost of variable
  // \param[in] variable pointer
  void rcfix_(RelaxationPtr rel, ModVector &r_mods,
             double bestobj,  const double rel_obj, double xval, double r,
             VariablePtr v);
};
  typedef RCHandler* RCHandlerPtr;
}


#endif

