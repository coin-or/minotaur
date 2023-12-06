//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file CxQuadHandler.h
 * \brief Define the CxQuadHandler class for handling convex quadratic constraints.
 * \author Mustafa Vora, IIT Bombay
 */

#ifndef MINOTAURCXQUADRATICHANDLER_H
#define MINOTAURCXQUADRATICHANDLER_H

#include "Handler.h"
#include "Types.h"

namespace Minotaur
{
class CxQuadHandler : public Handler
{
public:
  /// Default constructor.
  CxQuadHandler(EnvPtr env);

  /// Destroy
  ~CxQuadHandler();

  // Does nothing.
  void relaxInitFull(RelaxationPtr rel, bool* is_inf);

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, bool* is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool* is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool* is_inf);

  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, bool& is_inf,
                  double& inf_meas);

  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                CutManager* cutman, SolutionPoolPtr s_pool, ModVector& p_mods,
                ModVector& r_mods, bool* sol_found, SeparationStatus* status);

  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector& x,
                              ModVector& mods, BrVarCandSet& cands,
                              BrCandVector&, bool& is_inf);

  // Implement Handler::getBrMod().
  ModificationPtr getBrMod(BrCandPtr cand, DoubleVector& x, RelaxationPtr rel,
                           BranchDirection dir);

  // Implement Handler::getBranches().
  Branches getBranches(BrCandPtr cand, DoubleVector& x, RelaxationPtr rel,
                       SolutionPoolPtr s_pool);

  // presolve.
  SolveStatus presolve(PreModQ* pre_mods, bool* changed, Solution** sol);

  // Implement Handler::presolveNode().
  bool presolveNode(RelaxationPtr rel, NodePtr node, SolutionPoolPtr s_pool,
                    ModVector& p_mods, ModVector& r_mods);

  // Write name
  std::string getName() const;

private:
  // Absolute Tolerance
  double aTol_;

  // The logger
  LoggerPtr logger_;

  static const std::string me_;

  // Number of cuts generated
  UInt ncuts_;

  // Relative Tolerance
  double rTol_;
};

/// Shared pointer to CxQuadHandler.
typedef CxQuadHandler* CxQuadHandlerPtr;
} // namespace Minotaur
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
