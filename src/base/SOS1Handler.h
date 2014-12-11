//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file SOS1Handler.h
 * \brief Declare the SOS1Handler class for handling SOS type I constraints.
 * It checks integrality and provides branching candidates. Does
 * not do any presolving and cut-generation.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURSOS1HANDLER_H
#define MINOTAURSOS1HANDLER_H

#include "Handler.h"

namespace Minotaur {

/**
 * SOS1Handler class considers all SOS Type 1 constraints in the problem. 
 * It does not implement separate() function.  Implements functions for
 * checking feasibility and branching. 
 */
class SOS1Handler : public Handler {
public:

  /// Default constructor.
  SOS1Handler();

  /// Constructor.
  SOS1Handler(EnvPtr env, ProblemPtr problem);

  /// Destroy.
  ~SOS1Handler();

  // Does nothing.
  void relaxInitFull(RelaxationPtr rel, Bool *is_inf);

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, Bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, Bool *is_inf);

  // Check if solution is feasible.
  Bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  Bool & isInfeasible);

  /// Empty method as we do not do any separation in SOS1.
  void separate(ConstSolutionPtr, NodePtr , RelaxationPtr, CutManager *,
                SolutionPoolPtr, Bool *, SeparationStatus *);

  // Base class method.
  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                              ModVector & mods, BrVarCandSet &cands,
                              BrCandVector &gencands, bool & is_inf);

  // Base class method.
  ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x, 
                           RelaxationPtr rel, BranchDirection dir);

  //  Base class method.
  Branches getBranches(BrCandPtr cand, DoubleVector & x,
                       RelaxationPtr rel, SolutionPoolPtr s_pool);
      
  Bool isGUB(SOS *sos);
  Bool isNeeded();

  /// Presolve. Do not do any presolving.
  SolveStatus presolve(PreModQ *, Bool *) {return Finished;};

  /// Does nothing.
  void postsolveGetX(const Double *, UInt, DoubleVector *);

  /// Does nothing.
  Bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  // Write name
  std::string getName() const;

  void getNzNumSum_(SOSPtr sos, const DoubleVector x, int *nz,
                    double *nzsum);

  /// Does nothing.
  void writePreStats(std::ostream &) const {};

  /// Return the integer tolerance.
  Double getTol() const;

  /// Set the integer tolerance.
  void setTol(Double tol);

private:
  /// Environment.
  EnvPtr env_;

  /// Log
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// The problem for which the handler was created.
  ProblemPtr problem_;

  /**
   * \brief Tolerance for checking integrality.
   *
   * If abs(x) < zTol_, then it is considered to be zero.
   */
  Double zTol_;
};
typedef boost::shared_ptr<SOS1Handler> SOS1HandlerPtr;
typedef boost::shared_ptr<const SOS1Handler> ConstSOS1HandlerPtr;
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
