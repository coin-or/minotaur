// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
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
typedef boost::shared_ptr <Environment> EnvPtr;

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
  SolveStatus presolve(PreModQ *, bool *) 
  { return Finished; };

  // Base class method
  bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  // Base class method.
  void postsolveGetX(const double *, UInt, DoubleVector *) {};

  // Base class method 
  void relaxInitFull(RelaxationPtr, bool *) {};

  // Base class method. calls relax_().
  void relaxInitInc(RelaxationPtr, bool *) {};

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
  /// Pointer to environment's logger
  LoggerPtr logger_;

  /// Vector of duals of variables of the root relaxation
  double *rootDuals_;

  // TODO: Document what these members are
  double *rootPrimal_;
  double rootValue_;
  RCStats *stats_;
  const Timer* timer_;

  void copyRootDetails_(ConstSolutionPtr sol,  RelaxationPtr rel); 

  static const std::string me_;
};

  typedef boost::shared_ptr <RCHandler> RCHandlerPtr;
}


#endif

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
