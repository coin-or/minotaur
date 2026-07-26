//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file PowHandler.h
 * \brief Implement the handler for functions of type
 * y = x^k
 * \author Neeraj Kumar, IIT Bombay
 */

#ifndef MINOTAURPOWHANDLER_H
#define MINOTAURPOWHANDLER_H

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

  class PowHandler : public Handler {
  public:
    // Constructor
    PowHandler(EnvPtr env, ProblemPtr problem);
    PowHandler(EnvPtr env, ProblemPtr problem, ProblemPtr orig_);
    
    // Destructor
    ~PowHandler();

    void addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar,
                       ConstVariablePtr ovar, double degree, char sense = 'E');
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

  //  
     enum PowCurvature { PC_CONVEX, PC_CONCAVE, PC_MIXED };
 
    enum PowType {
      POS_OO,
      POS_OE,
      POS_EO, 
      NEG_OO,  
      NEG_OE,
      NEG_EO,
    };
/// Max number of two-point cuts any single branch emits.
/// buildPosEO_LT1_'s crossing-zero branch is the largest: 3.
static const size_t POW_NUM_PT_SLOTS = 3;

/// Max number of tangent cuts any single branch emits.
static const size_t POW_NUM_TAN_SLOTS = 3;
    struct PowCons {
      ConstraintPtr con;         
      ConstVariablePtr iv;       
      ConstVariablePtr ov;
      double k;
      int p;
      int q;
      PowType type;
      VariablePtr riv;           
      VariablePtr rov;           
      char sense;                
      std::vector<ConstraintPtr> ptCons;    // cuts through two points, by slot
      std::vector<ConstraintPtr> tanCons;   // cuts tangent at a point, by slot
      PowCons(ConstraintPtr newcon, ConstVariablePtr ivar,
                ConstVariablePtr ovar, double degree, int num, int den, PowType t, char s)
        : con(newcon), iv(ivar), ov(ovar), k(degree), p(num), q(den) , type(t) ,riv(0), rov(0),
          sense(s) {}
    };
 PowCurvature getCurvature_(const PowCons &cd, double xlb, double xub) const;

    typedef PowCons *PowConsPtr;
    typedef std::vector<PowConsPtr> PowConsVec;
    typedef PowConsVec::iterator PowConsIter;

    PowConsVec consd_;
    static const std::string me_;
bool sepIsGlobal_(const PowCons &cd) const;
    struct SepaStats {
      int iters;       
      int tangentcuts; 
      int optcuts;     
      int optrem;      
      double time;     
    };

    struct PowPresolveStats {
      UInt iters;
      double time;
      UInt conDel;
      UInt vBnd;
      UInt nMods;
      PowPresolveStats() : iters(0), time(0.0), conDel(0), vBnd(0), nMods(0) {}
    };

    struct BoundTighteningStats {
      int niters;
      int nLP;
      int dlb;
      int dub;
      double timeLP;
    };

    BoundTighteningStats bStats_;
    PowPresolveStats pStats_;
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


private:
  void relaxSlot_(PowCons &cd, RelaxationPtr rel,
                std::vector<ConstraintPtr> &slots, size_t slot,
                ModVector &mods);

  void relaxSlotsFrom_(PowCons &cd, RelaxationPtr rel,
                     std::vector<ConstraintPtr> &slots,
                     size_t from, size_t to, ModVector &mods); 
  void addCutByTan_(PowCons &cd, RelaxationPtr rel, double xv, int bound_dir,
                  ModVector &mods, size_t slot);
  void addCutByPts_(PowCons &cd, RelaxationPtr rel, double xlb, double xub,
                  int bound_dir, ModVector &mods, bool init, size_t slot);
  void linearize_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);

  void buildPosEO_GT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  void buildPosEO_LT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  
  void buildPosOO_GT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  void buildPosOO_LT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  
  void buildPosOE_GT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  void buildPosOE_LT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  
  void buildNegEO_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  void buildNegOO_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);
  void buildNegOE_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init);


  double computeWStar_(double k) const; // Newton-Raphson solver

  double computeWCross_(double k) const; 









    
  //Add cuts after separate_ function is called 
    void addCut_(VariablePtr x, VariablePtr y, double xval, double yval, 
                 RelaxationPtr rel, bool& ifcuts, PowCons *cd); 

  //Add Secant at root and other node for y=1/x
    void addSecant_(PowCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                    ModVector &mods, bool init);
  // delete duplicate constraint
    void dupRows_(bool *changed);
  //select branching point for a variable based on its domain 
    BranchPtr doBranch_(BranchDirection UpOrDown, ConstVariablePtr v, double bvalue);
  // returns violation in y-1/x
    double getViol_(const PowCons &cd, const DoubleVector &x) const;
     //Check Feasibility of y=1/x
    bool isFeasible_(ConstSolutionPtr sol, RelaxationPtr relaxation,
                     bool &should_prune, double &inf_meas);
    // Creates root relaxation
    void initRelax_(PowCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                    DoubleVector &grad);
    //from bound on y->x to bound on x->y
    bool propPowBnds_(PowConsPtr lcd, bool *changed);

    bool propPosEvenBnds_(PowConsPtr cd, bool *changed);
    bool propPosMonotonicBnds_(PowConsPtr cd, bool *changed);
    bool propNegEvenBnds_(PowConsPtr cd, bool *changed);
    bool propNegMonotonicBnds_(PowConsPtr cd, bool *changed);
    //marks dup row for deletion
    bool treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double mult,
                       bool *changed);
    // Update the node relaxation by changing the bound 
    void updateRelax_(PowCons &cd, RelaxationPtr rel, DoubleVector &tmpX,
                      DoubleVector &grad, ModVector &mods);

    int updatePBnds_(VariablePtr p, double newlb, double newub, bool *changed);
    int updatePBnds_(VariablePtr v, double newlb, double newub,
                     RelaxationPtr rel, bool mod_rel, bool *changed,
                     ModVector &p_mods, ModVector &r_mods);

    bool varBndsFromCons_(bool *changed);
    // Mathematical Evaluators
    double getF_(double xv, double k, PowType type) const;
    double getDf_(double xv, double k, PowType type) const;
    double getD2f_(double xv, double k, PowType type) const;
    
    // Safety and Validation Helpers
    double safeX_(double xv, double k) const;

    
    // Tangent Helpers
    double getMid_(double xlb, double xub, double k, PowType type) const;
    void addTan_(PowCons &cd, double xv, RelaxationPtr rel);

    // Secant Helpers
    bool prepSecantDomain_(double &xlb, double &xub, double k, PowType type) const;
    void applySecantCut_(PowCons &cd, double xlb, double xub, RelaxationPtr rel, 
                         ModVector &mods, bool init);
  };

}  // namespace Minotaur
#endif  // MINOTAURPOWHANDLER_H
