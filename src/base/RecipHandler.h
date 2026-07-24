//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file RecipHandler.h
 * \brief Implement the handler for functions of type
 * y = 1/x
 * \author Neeraj Kumar, IIT Bombay
 */

#ifndef MINOTAURRECIPHANDLER_H
#define MINOTAURRECIPHANDLER_H

#include "Handler.h"
#include "Types.h"

namespace Minotaur {

  class Engine;
  class Function;
  class LinearFunction;
  class Objective;
  class Problem;
  typedef LinearFunction *LinearFunctionPtr;
  typedef Objective *ObjectivePtr;

  class RecipHandler : public Handler {
  public:
    // Constructor
    RecipHandler(EnvPtr env, ProblemPtr problem);
    RecipHandler(EnvPtr env, ProblemPtr problem, ProblemPtr orig_);
    
    // Destructor
    ~RecipHandler();

    void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                       ConstVariablePtr ovar, char sense = 'E');
    //base method
    void addConstraint(ConstraintPtr) { assert(0); }
    
    //Does nothing
    void relaxInitFull(RelaxationPtr, SolutionPool *, bool *) {}
    //Does Nothing
    void relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf);
    bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation,
                    bool &should_prune, double &inf_meas);
    //Checks if Cutting plane can be added or not based on convexity/concavity
    void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel,
                  CutManager *cutman, SolutionPoolPtr s_pool,
                  ModVector &p_mods, ModVector &r_mods, bool *sol_found,
                  SeparationStatus *status);

    void relaxNodeFull(NodePtr, RelaxationPtr, bool *) {}

  //Update the relaxation at each node
    void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *isInfeasible);

    void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                                ModVector &mods, BrVarCandSet &cands,
                                BrCandVector &gencands, bool &is_inf);

    ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x,
                             RelaxationPtr rel, BranchDirection dir);

    Branches getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel,
                         SolutionPoolPtr s_pool);

    SolveStatus presolve(PreModQ *pre_mods, bool *changed, Solution **sol);

    bool presolveNode(RelaxationPtr p, NodePtr node, SolutionPoolPtr s_pool,
                      ModVector &p_mods, ModVector &r_mods);

    std::string getName() const override;
    void writeStats(std::ostream &out) const override;

  private:
    struct RecipCons {
      ConstraintPtr con;         
      ConstVariablePtr iv;       
      ConstVariablePtr ov;       
      VariablePtr riv;           
      VariablePtr rov;           
      char sense;                
      ConstraintPtr secCon;      
      ConstraintVector linCons;  

      RecipCons(ConstraintPtr newcon, ConstVariablePtr ivar,
                ConstVariablePtr ovar, char s)
        : con(newcon), iv(ivar), ov(ovar), riv(0), rov(0),
          sense(s), secCon(0), linCons() {}
    };

    typedef RecipCons *RecipConsPtr;
    typedef std::vector<RecipConsPtr> RecipConsVec;
    typedef RecipConsVec::iterator RecipConsIter;

    RecipConsVec consd_;
    static const std::string me_;

    struct SepaStats {
      int iters;       
      int tangentcuts; 
      int optcuts;     
      int optrem;      
      double time;     
    };

    struct RecipPresolveStats {
      UInt iters;
      double time;
      UInt conDel;
      UInt vBnd;
      UInt nMods;
      RecipPresolveStats() : iters(0), time(0.0), conDel(0), vBnd(0), nMods(0) {}
    };

    struct BoundTighteningStats {
      int niters;
      int nLP;
      int dlb;
      int dub;
      double timeLP;
    };

    BoundTighteningStats bStats_;
    RecipPresolveStats pStats_;
    SepaStats sStats_;

    double LBd_;
    double UBd_;
    double bTol_;
    EnvPtr env_;
    ProblemPtr orig_;
    ConstraintVector optCuts_;
    double aTol_;
    double rTol_;
    double eTol_;
    double vTol_;
    ProblemPtr p_;
    LoggerPtr log_;
    DoubleVector tmpX_;
    DoubleVector grad_;
    
  //Add cuts after separate_ function is called 
    void addCut_(VariablePtr x, VariablePtr y, double xval, double yval, 
                 RelaxationPtr rel, bool& ifcuts); 
  // Add Tangent at root and other node for y=1/x
    void addLin_(RecipCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                 DoubleVector &grad, ModVector &mods, bool init);
  //Add Secant at root and other node for y=1/x
    void addSecant_(RecipCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                    ModVector &mods, bool init);
  // delete duplicate constraint
    void dupRows_(bool *changed);
  //select branching point for a variable based on its domain 
    BranchPtr doBranch_(BranchDirection UpOrDown, ConstVariablePtr v, double bvalue);
  // returns violation in y-1/x
    double getViol_(const RecipCons &cd, const DoubleVector &x) const;
     //Check Feasibility of y=1/x
    bool isFeasible_(ConstSolutionPtr sol, RelaxationPtr relaxation,
                     bool &should_prune, double &inf_meas);
    // Creates root relaxation
    void initRelax_(RecipCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                    DoubleVector &grad);
    //from bound on y->x to bound on x->y
    bool propRecipBnds_(RecipConsPtr lcd, bool *changed);
    //marks dup row for deletion
    bool treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double mult,
                       bool *changed);
    // Update the node relaxation by changing the bound 
    void updateRelax_(RecipCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                      DoubleVector &grad, ModVector &mods);

    int updatePBnds_(VariablePtr p, double newlb, double newub, bool *changed);
    int updatePBnds_(VariablePtr v, double newlb, double newub,
                     RelaxationPtr rel, bool mod_rel, bool *changed,
                     ModVector &p_mods, ModVector &r_mods);

    bool varBndsFromCons_(bool *changed);
  };

}  // namespace Minotaur
#endif  // MINOTAURRECIPHANDLER_H
