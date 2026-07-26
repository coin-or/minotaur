///
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file AbsHandler.h
 * \brief Declare the handler for functions of type y = |x|.
 * \author Neeraj Kumar, IIT Bombay
 */

#ifndef MINOTAURABSHANDLER_H
#define MINOTAURABSHANDLER_H

#include "Handler.h"
#include "Types.h"

namespace Minotaur {

class Environment;
class Problem;
class Logger;

typedef Environment* EnvPtr;
typedef Problem* ProblemPtr;
typedef Logger* LoggerPtr;
/// Stores data for a single y = |x| constraint.
struct AbsCons {
  ConstraintPtr con;
  VariablePtr iv;
  VariablePtr ov;
  VariablePtr riv;
  VariablePtr rov;
  char sense;

  ConstraintPtr secCon;
  std::vector<ConstraintPtr> linCons;

  AbsCons(ConstraintPtr c, VariablePtr i, VariablePtr o, char s)
    : con(c), iv(i), ov(o), riv(0), rov(0), sense(s), secCon(0)
  {}
};

typedef AbsCons* AbsConsPtr;
typedef std::vector<AbsConsPtr> AbsConsVector;
typedef AbsConsVector::iterator AbsConsIter;
typedef AbsConsVector::const_iterator AbsConsConstIter;

class AbsHandler : public Handler {
public:
  AbsHandler(EnvPtr env, ProblemPtr problem);
  ~AbsHandler();

  using Handler::addConstraint;
  void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                      ConstVariablePtr ovar, char sense);

  void relaxInitInc(RelaxationPtr rel, SolutionPool *s_pool, bool *is_inf);
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_infeasible);
// Full relaxation methods required by the Handler base class
  void relaxInitFull(RelaxationPtr rel, SolutionPool *sp, bool *is_inf) override;
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool* is_inf) override;
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, bool &isfeas,
                   double &inf_meas);

  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                CutManager *cutman, SolutionPoolPtr s_pool,
                ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                SeparationStatus *status);

  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                               ModVector &mods, BrVarCandSet &cands,
                               BrCandVector &gencands, bool &is_inf);

  ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x,
                            RelaxationPtr rel, BranchDirection dir);

  Branches getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel,
                        SolutionPoolPtr s_pool);

  SolveStatus presolve(PreModQ *pre_mods, bool *changed, Solution **sol);
  bool presolveNode(RelaxationPtr rel, NodePtr node, SolutionPoolPtr s_pool,
                     ModVector &p_mods, ModVector &r_mods);

  std::string getName() const;
  void writeStats(std::ostream &out) const;
private:
  static const std::string me_;

  double bTol_;
  EnvPtr env_;
  double aTol_;
  double rTol_;
  double eTol_;
  double vTol_;
  ProblemPtr p_;
  LoggerPtr log_;

  AbsConsVector consd_;

  void initRelax_(AbsCons &cd, RelaxationPtr rel);
  void updateRelax_(AbsCons &cd, RelaxationPtr rel, ModVector &mods);
  void addLin_(AbsCons &cd, RelaxationPtr rel);
  void addSecant_(AbsCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  double getViol_(const AbsCons &cd, const DoubleVector &x) const;
  bool propAbsBnds_(AbsConsPtr cdata, bool *changed);
  int updatePBnds_(VariablePtr p, double newlb, double newub, bool *changed);
  int updatePBnds_(VariablePtr v, double newlb, double newub, RelaxationPtr rel,  
                  bool mod_rel, bool *changed, ModVector &p_mods, ModVector &r_mods);
};

typedef AbsHandler* AbsHandlerPtr;

}  // namespace Minotaur

#endif
