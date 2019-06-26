//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file TransSep.h
 * \brief Declare TransSep class for detecting separable nonlinear
 * constraints in the problem. It considers nonlinear constraints of a problem
 * and checks whether they are separable. If a nonlinear constraint is 
 * separable, then it reformulates original problem by introducing extra 
 * variables. Original nonlinear constraint becomes linear and each separable
 * part constitutes a nonlinear constraint. This class also gives computational
 * graphs of nonlinear constraints corresponding separable parts of separable
 * constraints.
 * \Author Meenarli Sharma, Indian Institute of Technology Bombay. 
 */

#ifndef MINOTAURTRANSSEP_H
#define MINOTAURTRANSSEP_H

#include <stack>

#include "Types.h"
#include "NonlinearFunction.h"
#include "OpCode.h"
#include "CNode.h"

namespace Minotaur {

  class CGraph;
  class CNode;
  class LinearHandler;
  class Problem;
  typedef CGraph* CGraphPtr;
  typedef std::deque<CNode *> CNodeQ;

  class TransSep {
  public:

    /// Default constructor.
    TransSep();

    /** 
    * \brief Default Constructor.
    * \param [in] env Environment pointer.
    * \param [in] minlp The minlp for which separability is to detected 
    */
    TransSep(EnvPtr env, ProblemPtr problem);

    /// Destroy.
    ~TransSep();

    /**
     * Check constraints of the given problem for separability.
     */
    void candCons();

    /// Clear data structures if constraint is not separable
    void clearCont();

    /// Clear and populates information
    bool clearpopu(UInt *j, std::vector<UInt > * m);

    /// Generate computation graph 
    void createCG(std::vector<CGraphPtr> * cg, CNodeQ * dq);

 
    /// Perform Depth first search rooted at node n1 at iteration number j
    void depthFS(int j, CNode *n1, std::vector<UInt > * m);
   
    /// Explore further from  node n1 at iteration number j
    void explore(int j, CNode *n1, std::vector<UInt > * m, int opc);

    /// Generate final computation graph
    void finalCG(std::vector<CGraphPtr> * cg);

    /// Find separability of given problem
    void findSep();

    /// Get left child of node n1 
    CNode* getLchild(CNode * n1) {return tempN_[n1->getL()->getIndex()];}; 
    
    /// Get right child of node n1 
    CNode* getRchild(CNode * n1) {return tempN_[n1->getR()->getIndex()];}; 

    /// Return Ids of separable constraints in original problem
    std::vector<UInt > getSepConId() {return sepConId_;}

    /// Give separable status of problem. 
    /// 1 if problem is separable, 0 otherwise
    bool getStatus();

    /// Mark nodes visited. Used in generating computation graph of separable
    /// parts
    void markVis(UInt i);

    /// Merge current iteration with iteration mNum
    void merge(int mNum);

    /// Find iteration to merge the current iteration to
    int mergeIt(int j, int a, std::vector<UInt > * m);

    /// Check separability of objective function
    void objSepCheck();

    /// Check whether constraint is separable
    bool outCheck(const CNode * o);


    /// Populate information about separable part
    void populate();

    /// Populate nodes list
    void popuon(CNode * n1, int opc, int t);

    /// Populate tempN_ 
    void popuTempN(CGraphPtr c, CNode *n1, OpCode op, bool k);

    /// Generate computational graph of separable parts
    std::vector<CGraphPtr> sepCGraph(CNodeQ * dq);

    /**
     * Check if the nonlinear constraint is separable. 1 if separable
     * 0 if not separable
     */
    bool sepCheck();

    /// Populate initial list for depth first search rooted at node n1
    void tempPop(CNode *n1, std::stack<CNode *> * tempNodes);

    /// Update linear part of the constraint function
    void updateLin(VariablePtr v, double d);

    /// Add information of the node already visited
    bool visited(UInt *j, std::vector<UInt > * m, CNode * n1,
                 int opc);

   /// Write reformulated problem after separability detection 
    void writeProb();

  private:
    /// Environment.
    EnvPtr env_;

    /// Log
    LoggerPtr logger_;

    // For log:
    static const std::string me_;

    // Problem whose constraints are to be checked for separability detection.
    ProblemPtr problem_;

    double coeff_;
  
    /// Iteration number at which a node of the cgraph is visited first
    std::vector<int> itnum_;

    // Separable parts
    std::vector<CNodeQ > sepNodes_;

    // Opcodes of nodes in separable parts
    std::vector< std::deque< int> > sepOps_;

    // Variables in separable parts
    std::vector< std::vector< CNode *> > sepVars_;

    // Constant nodes in separable parts
    std::vector< std::vector< CNode *> > sepConst_;

    // Ids of the Constraint that are separable
    std::vector<UInt > sepConId_;

    // No. of new constraints added to the original problem after sep detection
    UInt newCons_;
    
    // No. of new variable added to the origina problem after sep detection
    UInt newVars_;

    /// Used in generating computational graph of separable parts
    std::vector<CNode *> tempN_;

    // True if objective is separable
    bool objSep_;

    // True if problem is separable 
    bool sepStatus_;

    LinearFunctionPtr lf_;
    double  ub_;
    double  lb_;

    // Indicates whether the function under consideration belongs to
    // consrtaint or objective. True for constraint, false for objective 
    bool f_; 
 
    CNodeQ snodes_;

    std::deque<int > sops_;

    std::vector<CNode *> svars_;

    std::vector<CNode *> sconst_;

    std::stack<CNode *> on_;

    std::stack<int > onop_;

    UInt nnode_;
 
    UInt nvar_;

  };
  typedef TransSep* TransSepPtr;
  typedef const TransSep* ConstTransSepPtr;
}

#endif // MINOTAURTRANSSEP_H

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
