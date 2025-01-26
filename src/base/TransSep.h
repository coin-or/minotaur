//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
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

#include "Constraint.h"
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
     * Check constraints of the given problem for separability. Constraints
     * are expected in the form f(x) <= b
     */
    //void checkCons(std::vector<ConstraintPtr> *sepCons);
    void checkCons();

    /// Generate computation graph 
    void createCG(std::vector<CGraphPtr> * cg, CNodeQ * dq, int *itnum,
                  CNode **tempN);
 
    bool coeffValue(CNode * n2, double *cv,bool *n, CNode **n1,
                    std::vector<UInt > *idx);

    void commonConsCheck(std::vector<double > *constCoeff);
    /// Perform Depth first search rooted at node n1 at iteration number j
    void depthFS(int j, CNode *n1, std::vector<UInt > * m, int *itnum, CNodeQ *sNodes,
                       std::deque<int > *sOps, std::vector<CNode *> *sVars,
                       std::vector<CNode *> *sConsts);
   

    //UInt eqnDirection(ConstraintPtr cptr); 
    /// Explore further from  node n1 at iteration number j
    bool explore(UInt *j, CNode *n1, std::vector<UInt > * m, int opc, int *itnum, UInt nvar, CNodeQ *sNodes,
                       std::deque<int > *sOps, std::vector<CNode *> *sVars,
                       std::vector<CNode *> *sConsts);

    /// Generate final computation graph
    void finalCG(std::vector<CGraphPtr > * cg, 
                 CNode **tempN,std::vector<double > *constCoeff, double *coeff,
                 std::vector<double > *hashVal1,double *x);

    /// Find separability of given problem
    void sepDetection();
    //void sepDetection(std::vector<ConstraintPtr> *sepCons);

    /// Return no. of vars added 
    UInt getNumVars() {return newVars_;}

    /// Mark nodes visited. Used in generating computation graph of separable
    /// parts
    void markVis(UInt i, int *itnum);
    
    bool ifRepeated(CGraphPtr nlf,LinearFunctionPtr lfnew,
                        CGraphPtr cg,
                       double constCoeff, double coeff, LinearFunctionPtr lf);
    void linearizeObj();

    /// Merge current iteration with iteration mNum
    void mergeItrInfo(int mNum, CNodeQ *sNodes, std::deque<int > *sOps,
                            std::vector<CNode *> *sVars,
                            std::vector<CNode *> *sConsts);

    /// Find iteration to merge the current iteration to
    int mergeItrNum(int j, int a, std::vector<UInt > * m);

           

    bool sepPartsSearch(UInt sp, std::vector<CGraphPtr > *nlfnew,
                              std::vector<LinearFunctionPtr > *lfnew,
                              CGraphPtr cg, double constCoeff,
                              LinearFunctionPtr lf, std::vector<UInt > *nsp,
                              double coeff);

    /// Check whether constraint is separable
    bool rootChildren(const CNode * o, std::stack<CNode *> *candNodes,
                      std::stack<int > *candOp,
                      double *coeff, UInt nnode, UInt nvar, double *ub,
                      //double *lb, LinearFunctionPtr lf);
                      LinearFunctionPtr lf);

    /// Generate computational graph of separable parts
    std::vector<CGraphPtr> sepPartsCGraph(CNodeQ * dq, UInt nnode,
                                          std::vector<double > *constCoeff, double *coeff,
                                          std::vector<double > *hashVal1, double *x);
    /**
     * Check if the nonlinear constraint is separable. 1 if separable
     * 0 if not separable
     */
    bool sepCheck(double coeff, std::stack<CNode *> *candNodes,
                      std::stack<int > *candOp, UInt nnode, UInt nvar,
                      //double *ub, double *lb, LinearFunctionPtr lf);
                      double *ub, LinearFunctionPtr lf);
    

    /// Populate initial list for depth first search rooted at node n1
    void tempPopulate(CNode *n1, std::stack<CNode *> * tempNodes);

    /// Add information of the node already visited

    void updateVisNodeItr(UInt* j, std::vector<UInt >*m, CNode *n1,
                                int opc, int idx, CNodeQ *sNodes,
                                std::deque<int > *sOps, 
                                std::vector<CNode *> *sVars,
                                std::vector<CNode *> *sConsts);
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

    // No. of new variable added to the origina problem after sep detection
    UInt newVars_;
  
    // Separable parts
    std::vector<CNodeQ > sepNodes_;

    // Name of the Constraint that are separable

    // Opcodes of nodes in separable parts
    std::vector< std::deque< int> > sepOps_;

    // Variables in separable parts
    std::vector< std::vector< CNode *> > sepVars_;

    // Constant nodes in separable parts
    std::vector< std::vector< CNode *> > sepConst_;

    std::vector<ConstraintPtr> sepCons_;

    double intTol_;
  };
  typedef TransSep* TransSepPtr;
  typedef const TransSep* ConstTransSepPtr;
}

#endif // MINOTAURTRANSSEP_H

