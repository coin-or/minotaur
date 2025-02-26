//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2009 - 2025 The Minotaur Team.
//

/**
 * \file Types.h
 * \brief Declare important 'types' used in Minotaur.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURTYPES_H
#define MINOTAURTYPES_H

#include <assert.h>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <stdlib.h>
#include <string>
#include <vector>

namespace Minotaur
{

/// Unsigned integer
typedef unsigned int UInt;

/// Containers for standard types
typedef std::deque<UInt> UIntQ;
typedef std::vector<bool> BoolVector;
typedef std::vector<double> DoubleVector;
typedef std::vector<int> IntVector;
typedef std::vector<UInt> UIntVector;
typedef std::set<UInt> UIntSet;

/// Different types of files for reading/writing
typedef enum { MPS, NL, FileTypeNone } FileType;

/// The different classes of problems
typedef enum {
  LP,
  MILP,
  QP,
  MIQP,
  QCQP,
  MIQCQP,
  POLYP,
  MIPOLYP,
  NLP,
  MINLP,
  OtherProblemType
} ProblemType;

/**
 * Objective sense. Minotaur minimizes. Always. If the sense is to Maximize,
 * then Minotaur converts the objective.
 */
typedef enum { Minimize, Maximize } ObjectiveType;

/// Different types of functions in Minotaur.
typedef enum {
  Constant,
  Linear,
  Multilinear,
  Quadratic,
  Nonlinear,
  Polynomial,
  OtherFunctionType
} FunctionType;

/// Different types of variables.
typedef enum {
  Binary,    ///< Variable is constrained to be binary.
  Integer,   ///< Variable is constrained to be integer.
  ImplBin,   ///< Variable is continuous, but will take binary values only.
  ImplInt,   ///< Variable is continuous, but will take integer values only.
  Continuous ///< Variable is continuous.
} VariableType;

/// Where did the variable originate from?
typedef enum {
  VarOrig,    ///> Variable was part of the original problem
  VarTran,    ///> Variable created by Transformer
  VarSep,     ///> Variable created by TransSep for separable constraints
  VarHand,    ///> Variable created by Handler
  VarOtherSrc ///> Variable created by some other source
} VarSrcType;

/// Where did the constraint originate from?
typedef enum {
  ConsOrig, ///> Constraint was part of the original problem
  ConsTran, ///> Constraint added by Transformer
  ConsTranOrig, ///> Constraint added by Transformer as a reformulation of original constraint
  ConsHand, ///> Constraint added by Handler
  ConsCut   ///> Constraint added as a cut
} ConsSrcType;

/// What type of constraint is this.
/// Add enum items here if more constraint types are identified.
typedef enum {
  Aggregation,   ///> ax + by = c
  Precedence,    ///> ax - ay \leq b, x & y have the same type
  VariableBound, ///> ax + by \leq c, x is Binary
  SetPartition,  ///> \sum_{i=1}^n x_i = 1, x_i is Binary
  SetPack,       ///> \sum_{i=1}^n x_i \leq 1, x_i is Binary
  SetCover,      ///> \sum_{i=1}^n x_i \geq 1, x_i is Binary
  Cardinality,   ///> \sum_{i=1}^n x_i = k, x_i is Binary, k \geq 2
  InvariantKnapsack, ///> \sum_{i=1}^n x_i \leq b, x_i is Binary, b \in \mathbb{N} \geq 2
  EquationKnapsack, ///> \sum_{i=1}^n a_i*x_i = b, x_i is Binary, b \in \mathbb{N} \geq 2
  BinPack, ///> \sum_{i=1}^n a_i*x_i + by = b, x_i, y is Binary, b \in \mathbb{N} \geq 2
  Knapsack,        ///> \sum_{i=1}^n a_i*x_i \leq b, x_i is Binary
  IntegerKnapsack, ///> \sum_{i=1}^n a_i*x_i \leq b, x_i is Integer
  MixedBinary, ///> Linear combination of at least one binary variable with continuous variables
  General,      ///> No specific structure identififed
  Quad,        ///> Quadratic
  DiagQuad,    ///> Purely Quadratic with all zero elements of hessian except diagonal elements
  SimpleBall,  ///> \sum_{i=1}^n x_i^2 \leq b, b \geq 0
  Ellipsoid,    ///> \sum_{i=1}^n a_i*x_i^2 \leq b, b \geq 0, a_i \geq 0
  ComplementSimpleBall, ///> Complement of SimpleBall
  ComplementEllipsoid,   ///> Complement of Ellipse 
  PureBilinear, ///> \sum_{i=1}^n x_i*x_{i+a} \leq b or \geq b, where a \in \mathbb{Z}^{+}
  SOCRepresentableQuad, ///> Quadratic constraint which can be represented as SOC constraint
  NonLin       ///>Non-linear
} ConsType;

/// Different types of variable-bounds.
typedef enum { Lower, Upper } BoundType;

/// SOS types
typedef enum { SOS1, SOS2 } SOSType;

/// Different states a variable can be in.
typedef enum {
  DeletedVar, ///< Marked for deletion.
  FixedVar,   ///< Fixed to a value.
  FreeVar,    ///< Doesn't have bounds. Same as superbasic in LP
  NormalVar   ///< Not in any other category
} VarState;

/// Different states a constraint can be in.
typedef enum {
  DeletedCons, ///< Marked for deletion.
  FreeCons,    ///< Doesn't have bounds. Implies redundant.
  NormalCons   ///< Not in any other category
} ConsState;

/// Different states an objective can be in.
typedef enum {
  DeletedObj, ///< Marked for deletion.
  NormalObj   ///< Not in any other category
} ObjState;

/// Different states an algorithm like branch-and-bound can be in.
typedef enum {
  NotStarted,
  Started,
  Restarted,
  SolvedOptimal,
  SolvedInfeasible,
  SolvedUnbounded,
  SolvedGapLimit,
  SolvedSolsLimit,
  IterationLimitReached,
  Interrupted,
  TimeLimitReached,
  SolLimitReached,
  SolveError,
  Finished
} SolveStatus;

/// Different status that an external engine may report.
typedef enum {
  ProvenOptimal,
  ProvenLocalOptimal,
  ProvenInfeasible,
  ProvenLocalInfeasible,
  ProvenUnbounded,
  ProvenObjectiveCutOff,
  EngineIterationLimit,
  ProvenFailedCQFeas,
  ProvenFailedCQInfeas,
  FailedFeas,
  FailedInfeas,
  EngineError,
  EngineUnknownStatus
} EngineStatus;

/// What can a brancher do to a node in branch-and-bound.
typedef enum {
  ModifiedByBrancher,
  PrunedByBrancher,
  NoCandToBranch,
  NotModifiedByBrancher
} BrancherStatus;

/// Two directions for branching
typedef enum { DownBranch, UpBranch, OtherBranchDir } BranchDirection;

/// Status of a node in branch-and-bound.
typedef enum {
  NodeNotProcessed, //< has been created but not yet been processed.
  NodeInfeasible,   //< can be pruned
  NodeHitUb,        //< can be pruned
  NodeDominated,    //< can be pruned
  NodeOptimal,      //< The solution obtained is integral, feasible and optimal
  //< for the subtree. can prune.
  NodeContinue, //< solution is neither optimal, nor the relaxation
  //< provably infeasible.
  NodeStopped
} NodeStatus;

/// Status from separation routine:
typedef enum {
  SepaContinue, ///< Separation routine wants nothing.
  SepaResolve,  ///< Separation routine wants the relaxation resolved.
  SepaPrune,    ///< No need to further solve the subproblem.
  SepaNone,     ///< No separation found.
  SepaError     ///< Problem separating a point.
} SeparationStatus;

/// Levels of verbosity.
typedef enum {
  LogNone,      //< absolutely no messages
  LogError,     //< only error messages and warnings if any
  LogInfo,      //< errors, timed status, final solution
  LogExtraInfo, //< some extra info, stats etc
  LogDebug,     //< more verbose
  LogDebug1,    //< more verbose
  LogDebug2     //< more verbose
} LogLevel;

/// Order of tree search.
typedef enum {
  DepthFirst,
  BestFirst,
  BestThenDive /// First find the best bound, then dive until pruned.
} TreeSearchOrder;

/// Convexity of a function or a constraint.
typedef enum {
  Convex,
  Concave, /// Constraint can't be concave only functions can be concave.
  Nonconvex,
  Unknown
} Convexity;

// 1=like_red, 2=blue, 4=red, 5=yellow, 6=black, 7=pink, 8=cyan, 9=green
// 11=orange, 12=green, 13=pink, 14=light blue
/// Colors for tree-visualization using vbc
typedef enum {
  VbcActive = 4,  /// Unsolved, open.
  VbcFeas = 2,    /// incumbent.
  VbcInf = 11,    /// infeasible.
  VbcSolved = 9,  /// solved.
  VbcSolving = 8, /// currently being solved.
  VbcSubInf = 13, /// subtree is infeasible.
  VbcSubOpt = 6   /// suboptimal.
} VbcColors;

/// Tags (int) for messages for distributed-memory (MPI) parallel tree-search
typedef enum {
  Terminate = 0,  /// Tree fully solved, terminate.
  NodeFound = 1,  /// Node is available, receive it.
  Wait = 2,       /// Wait untill further notice.
  UB = 3,       /// Upper bound is shared.
  LB = 4,       /// Lower bound is shared.
} MpiMessageTag;

// Miscellaneous functions related to above types.

/// What is the function type on adding two functions f1 and f2?
FunctionType funcTypesAdd(FunctionType f1, FunctionType f2);

/// What is the function type on multiplying two functions f1 and f2?
FunctionType funcTypesMult(FunctionType f1, FunctionType f2);

/// Get the string equivalent of ProblemType.
std::string getProblemTypeString(ProblemType p);

/// Get the string equivalent of FunctionType.
std::string getFunctionTypeString(FunctionType f);

/// Get the string equivalent of SolveStatus.
std::string getSolveStatusString(SolveStatus s);

// Important class definitions
/// Logging
class Logger;
typedef Logger* LoggerPtr;

class Variable;
typedef Variable* VariablePtr;
typedef Variable* ConstVariablePtr;
typedef std::vector<VariablePtr> VarVector;
typedef std::deque<VariablePtr> VarQueue;
typedef VarVector::const_iterator VariableConstIterator;
typedef VarVector::iterator VariableIterator;
typedef std::vector<ConstVariablePtr>::iterator ConstVarIter;
typedef VarQueue::const_iterator VarQueueConstIter;
typedef VarQueue::iterator VarQueueIter;
struct CompareVariablePtr {
  bool operator()(ConstVariablePtr v1, ConstVariablePtr v2) const;
};
typedef std::set<ConstVariablePtr, CompareVariablePtr> VariableSet;
typedef VariableSet::const_iterator VarSetConstIterator;
typedef std::set<VariablePtr> VarSet;

class Objective;
typedef Objective* ObjectivePtr;
typedef const Objective* ConstObjPtr;

class Constraint;
typedef Constraint* ConstraintPtr;
typedef const Constraint* ConstConstraintPtr;
typedef std::vector<ConstraintPtr> ConstraintVector;
typedef ConstraintVector::iterator ConstraintIterator;
typedef ConstraintVector::const_iterator ConstraintConstIterator;

class SOS;
typedef SOS* SOSPtr;
typedef std::vector<SOS*> SOSVector;
typedef SOSVector::iterator SOSIterator;
typedef SOSVector::const_iterator SOSConstIterator;

// Serdar defined
typedef ConstraintVector* ConstraintVectorPtr;
typedef std::vector<ConstConstraintPtr> ConstConstraintVector;
typedef ConstConstraintVector* ConstConstraintVectorPtr;
typedef ConstConstraintVector::iterator ConstConstraintIterator;
typedef std::set<ConstraintPtr> ConstrSet;
typedef std::deque<ConstraintPtr> ConstrQ;

// Serdar added.
typedef VarSet* VarSetPtr;
typedef const VarSet* ConstVarSetPtr;
typedef std::map<ConstVariablePtr, UInt, CompareVariablePtr> VarIntMap;
typedef VarIntMap::const_iterator VarIntMapConstIterator;
typedef VarIntMap::iterator VarIntMapIterator;

typedef VarSet::iterator VarSetIter;
typedef VarSet::const_iterator VarSetConstIter;
typedef std::set<ConstVariablePtr> ConstVarSet;
typedef VarSet::const_iterator ConstVarSetIter;

class Node;
typedef Node* NodePtr;
typedef std::vector<NodePtr> NodePtrVector;
typedef std::vector<NodePtr>::iterator NodePtrIterator;

class Handler;
typedef Handler* HandlerPtr;
typedef const Handler* ConstHandlerPtr;
typedef std::vector<HandlerPtr> HandlerVector;
typedef HandlerVector::iterator HandlerIterator;
typedef HandlerVector::const_iterator HandlerConstIterator;

class Environment;
typedef Environment* EnvPtr;

class Problem;
typedef Problem* ProblemPtr;
typedef const Problem* ConstProblemPtr;

class Modification;
typedef Modification* ModificationPtr;
typedef const Modification* ConstModificationPtr;
typedef std::vector<ModificationPtr> ModVector;
typedef std::vector<ModificationPtr>::const_iterator ModificationConstIterator;
typedef std::vector<ModificationPtr>::const_reverse_iterator
    ModificationRConstIterator;
typedef std::deque<ModificationPtr> ModQ;
typedef std::stack<ModificationPtr> ModStack;

class Branch;
typedef Branch* BranchPtr;
typedef const Branch* ConstBranchPtr;
typedef std::vector<BranchPtr> BranchPtrVector;
typedef BranchPtrVector* Branches;
typedef std::vector<BranchPtr>::const_iterator BranchConstIterator;

class BrCand;
typedef BrCand* BrCandPtr;
typedef const BrCand* ConstBrCandPtr;
typedef std::vector<BrCandPtr> BrCandVector;
typedef BrCandVector::iterator BrCandVIter;

class BrVarCand;
typedef BrVarCand* BrVarCandPtr;
typedef const BrVarCand* ConstBrVarCandPtr;
struct CompareVarBrCand {
  bool operator()(ConstBrVarCandPtr c1, ConstBrVarCandPtr c2) const;
};
typedef std::set<BrVarCandPtr, CompareVarBrCand> BrVarCandSet;
typedef BrVarCandSet::iterator BrVarCandIter;
typedef std::vector<BrVarCandPtr>::iterator BrVarCandVIter;

// Serdar added this block
// Compare some pairs.
typedef std::pair<int, double> id;
struct CompareIntDouble {
  bool operator()(id id1, id id2) const;
};

// Compare rule to compare the value of the pair.
typedef std::pair<ConstVariablePtr, double> VariableValuePair;
typedef VariableValuePair* VariableValuePairPtr;
struct CompareValueVariablePair {
  bool operator()(VariableValuePair v1, VariableValuePair v2) const;
};

// Map to determine GUB cover.
struct CompareValueVarInc {
  bool operator()(VariableValuePair v1, VariableValuePair v2) const;
};
typedef std::map<ConstVariablePtr, double, CompareValueVarInc> VariableValueMap;

// Vector of pair<ConstVariablePtr,double value> which is VariableValuePair.
typedef std::vector<VariableValuePair> VariableValuePairVector;
typedef VariableValuePairVector* VariableValuePairVectorPtr;
typedef std::vector<VariableValuePair>::iterator
    VariableValuePairVectorIterator;
typedef std::vector<VariableValuePair>::const_iterator
    VariableValuePairVectorConstIterator;

// Cover is a VariableSet
typedef VariableValuePairVector CoverSet;
typedef CoverSet* CoverSetPtr;
typedef const CoverSet* ConstCoverSetPtr;
typedef CoverSet::iterator CoverSetIterator;
typedef CoverSet::const_iterator CoverSetConstIterator;

// Pointers to classes added.
class KnapsackList;
typedef KnapsackList* KnapsackListPtr;
typedef const KnapsackList* ConstKnapsackListPtr;

// Cut class vectors etc.
class Cut;
typedef Cut* CutPtr;
typedef std::vector<CutPtr> CutVector;
typedef CutVector::iterator CutVectorIter;
typedef CutVector::const_iterator CutVectorConstIter;
typedef std::list<CutPtr> CutList;
typedef CutList::iterator CutListIter;

//LiftingProblem which is a knapsack problem type of Problem.
typedef Problem LiftingProblem;
typedef ProblemPtr LiftingProblemPtr;

// CoverCutGenerator pointer
class CoverCutGenerator;
typedef CoverCutGenerator* CoverCutGeneratorPtr;

// LGCIGenerator pointer
class LGCIGenerator;
typedef LGCIGenerator* LGCIGeneratorPtr;
// Serdar ended.

class Function;
typedef Function* FunctionPtr;
typedef const Function* ConstFunctionPtr;

class LinearFunction;
typedef LinearFunction* LinearFunctionPtr;
typedef const LinearFunction* ConstLinearFunctionPtr;

class NonlinearFunction;
typedef NonlinearFunction* NonlinearFunctionPtr;
typedef const NonlinearFunction* ConstNonlinearFunctionPtr;

class QuadraticFunction;
typedef QuadraticFunction* QuadraticFunctionPtr;
typedef const QuadraticFunction* ConstQuadraticFunctionPtr;

// declare options database
class OptionDB;
typedef OptionDB* OptionDBPtr;

// declare options
template <class T>
class Option;
typedef Option<bool> BoolOption;
typedef Option<int> IntOption;
typedef Option<double> DoubleOption;
typedef Option<std::string> StringOption;
typedef BoolOption* BoolOptionPtr;
typedef IntOption* IntOptionPtr;
typedef DoubleOption* DoubleOptionPtr;
typedef StringOption* StringOptionPtr;
typedef BoolOptionPtr FlagOptionPtr;

// define sets of pointers
typedef std::set<BoolOptionPtr> BoolOptionSet;
typedef std::set<IntOptionPtr> IntOptionSet;
typedef std::set<DoubleOptionPtr> DoubleOptionSet;
typedef std::set<StringOptionPtr> StringOptionSet;
typedef BoolOptionSet FlagOptionSet;

// define iterators of pointers
typedef BoolOptionSet::iterator BoolOptionSetIter;
typedef IntOptionSet::iterator IntOptionSetIter;
typedef DoubleOptionSet::iterator DoubleOptionSetIter;
typedef StringOptionSet::iterator StringOptionSetIter;
typedef BoolOptionSetIter FlagOptionSetIter;

/// Variables should always be constant within a group
typedef std::map<ConstVariablePtr, double, CompareVariablePtr> VariableGroup;
typedef std::map<ConstVariablePtr, double>::iterator VariableGroupIterator;
typedef std::map<ConstVariablePtr, double>::const_iterator
    VariableGroupConstIterator;

/// Pairs of variables are used in quadratic functions.
typedef std::pair<ConstVariablePtr, ConstVariablePtr> VariablePair;
typedef std::pair<ConstVariablePtr, ConstVariablePtr> ConstVariablePair;

struct CompareVariablePair {
  bool operator()(ConstVariablePair tv1, ConstVariablePair tv2) const;
};

/// Again, Variables should always be constant within a group
typedef std::map<ConstVariablePair, double, CompareVariablePair>
    VariablePairGroup;
typedef std::map<ConstVariablePair, UInt, CompareVariablePair> VarPairIntMap;
typedef std::map<VariablePair, double>::iterator VariablePairGroupIterator;
typedef std::map<ConstVariablePair, double>::const_iterator
    VariablePairGroupConstIterator;
typedef std::map<ConstVariablePtr, ConstVariablePtr, CompareVariablePtr>
    VarVarMap;
typedef VarVarMap::const_iterator VarVarMapConstIterator;

typedef const std::map<ConstVariablePtr, UInt, CompareVariablePtr>
    VarCountConstMap;

class Heuristic;
typedef Heuristic* HeurPtr;
typedef std::vector<HeurPtr> HeurVector;
} // namespace Minotaur

#endif

