//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file TransSep.h
 * \brief Declare the SeparabilityDectector class for detecting nonlinear
 * function separability. It checks whether the given nonlinear function
 * is separable. If separable, then it reformulates the original problem.
 * It also gives separable parts and computational graph of each separable part.
 * \author Meenarli Sharma, Indian Institute of Technology Bombay. 
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
  typedef boost::shared_ptr<CGraph> CGraphPtr;
  typedef boost::shared_ptr<Environment> EnvPtr;
  typedef boost::shared_ptr<Problem> ProblemPtr;
  typedef std::deque<CNode *> CNodeQ;

  //MS: do we want these statistics? 
  //struct SepStats {                                     i
    //size_t sCons;      /// Number of separable constraints.                            
    //std::cout vector<int >  sParts;  /// Number of separable parts
                                    ///// in each separable constraint.                  
    //size_t newNlC;    /// Number of new nonlinear constraints added to the problem.
    //size_t newLC;     /// Number of new linear constraints added to the problem. 
                      /////This is equal to sCons.
    //size_t delC;      /// Number of constraints deleted from the problem.
                      ///// This must be equal to sCons and newLC.                              
  //};  
  /**
   * TransSep class considers nonlinear constraints of a problem and 
   * checks whether it is separable. If separable, then it reformulates the
   * original problem by introducing extra variables. Original nonlinear
   * constraint becomes linear and each separable part constitutes a nonlinear
   * constraint. This class also gives computational graph of each nonlinear
   * constraint corresponding to each separable part.
   */
  class TransSep {
  public:

    /// Default constructor.
    TransSep();

    /// Constructor.
    TransSep(EnvPtr env, ProblemPtr problem);

    /// Destroy.
    ~TransSep();

    /**
     * Consider constraints of the given problem one by one and if it is
     * nonlinear make it a candidate for separability detection
     * it is separable.
     */
    void candCons();

    /**
     *Check is the nonlinear constraint is separable
     */
    bool sepCheck(UInt nnode, CNodeQ * snodes, 
                  std::deque<int > * sops, std::vector<CNode *> *svars,
                  std::vector<CNode *> *sconst, UInt nvar,
                  std::stack<CNode *> *oN, std::stack<int > *oNop);

    //bool SepCheck(const CNode *node, UInt nnode, CNodeQ * snodes, 
                  //std::deque<int > * sops, std::vector<CNode *> *svars,
                  //std::vector<CNode *> *sconst, UInt nvar,
                  //std::stack<CNode *> *oN, std::stack<int > *oNop);

    /**
     *Depth first search rooted at node n1 at iteration number j
     */
    void DepthFS(int j, CNode *n1, CNodeQ * snodes, std::deque<int > * sops,
                 std::vector<CNode *> * svars, std::vector<CNode *> *sconst,
                 std::vector<UInt > * m);

    /**
     *To populate initial list for depth first search rooted at node n1
     */
    void TempPop(CNode *n1, std::stack<CNode *> * tempNodes);

    /**
     *Merge operations
     */
    int MergeIt(int j, int a, std::vector<UInt > * m);

    void Merge(int mNum, CNodeQ * snodes, std::deque<int > * sops, 
               std::vector<CNode *> *svars, std::vector<CNode *> *sconst);


    //cgraph of separable parts
    std::vector<CGraphPtr> SepCGraph(UInt nnode, CNodeQ * dq);
    
    void createCG(std::vector<CGraphPtr> * cg, CNodeQ * dq);
    void finalCG(std::vector<CGraphPtr> * cg);

    CNode* getLchild(CNode * n1) {return tempN_[n1->getL()->getIndex()];} ; 
    CNode* getRchild(CNode * n1) {return tempN_[n1->getR()->getIndex()];} ; 

    //Mark nodes visited
    void MarkVis(UInt i);

    bool outCheck(const CNode * o, UInt nnode, UInt nvar);

    //For problem display
    void disProb();

    //Objective separability check
    void objSepCheck();

  private:
    /// Environment.
    EnvPtr env_;

    /// Log
    LoggerPtr logger_;

    /// For log:
    static const std::string me_;

    /// Problem whose constraints are to be checked for separability detection.
    ProblemPtr problem_;

    //LinearFunctionPtr lf_;

    //double lb_;
    //double ub_;

    //bool lfmod_;

    double coeff_;
    bool bcoeff_;
    ///Separability status
    //bool sepStatus_;

    /// Store whether constraints in the problem are separable (1) 

    ///Stack of open nodes and opcode of their parents
    //std::stack<CNode *>  oN_;
    //std::stack<int>  oNop_;

    ///iteration number at which a node of the cgraph is visited first
    std::vector<int> itnum_;

    //Variable for merge operation
    //std::vector<UInt> m_;

    //Vector of vectors for storing separable parts
    std::vector< CNodeQ > SepNodes_;

    //Vector of vectors for storing opcodes of nodes in separable parts
    std::vector< std::deque< int> > SepOps_;

    //Vector of vectors for storing variables separable parts
    std::vector< std::vector< CNode *> > SepVars_;

    //Vector of vectors for storing constant nodes in separable parts
    std::vector< std::vector< CNode *> > SepConst_;


    // Total iterations
    //UInt j_;

    // No. of separable constraints in problem 
    UInt sepC_;

    //No. of new separable constraints added to the problems
    UInt newCons_;
    
    //No. of new variable added to the problem
    UInt newVars_;

    std::vector<CNode *> tempN_;

    //Is problem separable
    //bool sep_
 
    //Is objective separable
    bool objSep_;
        
    //No. of separable constraints function
    //UInt spCon_;

  };
  typedef boost::shared_ptr<TransSep> TransSepPtr;
  typedef boost::shared_ptr<const TransSep> ConstTransSepPtr;
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
