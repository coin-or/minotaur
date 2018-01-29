//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutHandler.h
 * \brief Declare PerspCutHandler class for perspective reformulation (PR).
 * It detects constraints amenable to perspective reformulation and generates
 * perspective cuts whenever needed. 
 * \author Meenarli Sharma, Indian Institute of Technology
 */

#ifndef MINOTAURPERSPCUTHANDLER_H
#define MINOTAURPERSPCUTHANDLER_H

#include "Handler.h"
#include "Problem.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"

namespace Minotaur {

  class PerspCutHandler;
  typedef boost::shared_ptr<PerspCutHandler> PerspCutHandlerPtr;
  typedef boost::shared_ptr<const PerspCutHandler> ConstPerspCutHandlerPtr;
  
  class PerspCutHandler : public Handler {
  public:

  /// Default constructor.
  PerspCutHandler();
  
  /// Constructor.
  PerspCutHandler(EnvPtr env, ProblemPtr problem);

  /// Destructor.
  ~PerspCutHandler();

  /// Generates perspective cuts.
  LinearFunctionPtr generateCut(UInt relvars, ConstSolutionPtr sol,
                                    ConstConstraintPtr c, ConstVariablePtr v); 
 
  /// Does nothing.
  virtual Branches getBranches(BrCandPtr, DoubleVector &,
                               RelaxationPtr, SolutionPoolPtr)
    {return Branches();}; 

  // Does nothing.
  virtual void getBranchingCandidates(RelaxationPtr,
                                      const DoubleVector &, ModVector &,
                                      BrVarCandSet &, BrCandVector &,
                                      bool &) {};

  /// Does nothing.
  virtual ModificationPtr getBrMod(BrCandPtr, DoubleVector &,
                                   RelaxationPtr, BranchDirection)
    {return ModificationPtr();};

  /// Returns name of the handler.
  std::string getName() const;

  /// Generates Perspective cuts
  LinearFunctionPtr gPCut(UInt relvars, ConstConstraintPtr c,
                          ConstVariablePtr v, double *y);

  /// Checks feasibility of constraints amenable to PR at the given solution.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation,
                  bool &should_prune, double &inf_meas);

  /// Generates a list of constraints amenable to PR.
  bool perspList();
 
  /// Does nothing.
  SolveStatus presolve(PreModQ *, bool *) {return Finished;};

  /// Does nothing.
  virtual bool presolveNode(RelaxationPtr, NodePtr,
                            SolutionPoolPtr, ModVector &,
                            ModVector &) {return false;};

  /// Does nothing.
  void relaxInitFull(RelaxationPtr, bool * ) {};
  
  /// Does nothing.
  void relaxInitInc(RelaxationPtr, bool * ) {};
  
  /// Does nothing.
  void relaxNodeFull(NodePtr, RelaxationPtr, bool * ) {};

  /// Does nothing.
  void relaxNodeInc(NodePtr, RelaxationPtr, bool * ) {};

  /// Separates current solution. 
  void separate(ConstSolutionPtr, NodePtr, RelaxationPtr, CutManager *cutman,
                SolutionPoolPtr, bool *, SeparationStatus * status);
 
  /// Writes statistics.
  void writeStats(std::ostream &out) const;
  
private:
  
  /// Environment.
  EnvPtr env_;
  
  /// Problem for which the handler is created.
  ProblemPtr minlp_;
  
  /// Log.
  LoggerPtr logger_;
 
  /// True if all constraints amenable to PR are satisfied at current solution
  /// else false.
  bool isFeas_;
  
  /// Tolerance for accepting a new solution value: absolute threshold.
  double solAbsTol_;
  
  /// Tolerance for accepting a new solution value: relative threshold.
  double solRelTol_;
  
  /// Tolerance for checking integrality.
  double intTol_;

  // Number of perspective cuts generated.
  UInt numCuts_;
  
  /// For log:
  static const std::string me_;
  
  /// Vector of pointers to perspective constraint.
  std::vector<ConstConstraintPtr> cons_;
  
  /// Vector of pointers to binary variables controlling constraints amenable
  /// to PR.
  std::vector<ConstVariablePtr> binvar_;

  /// Pointer to constraint amenable to PR.
  PerspConPtr persplist_;
  };

}

#endif // MINOTAURPERSPCUTHANDLER_H

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
