//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
//


/**
 * \file CGraphPers.h
 * \brief Declare class CGraphPers for representing perspective of a nonlinear
 * function.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURCGRAPHPERS_H
#define MINOTAURCGRAPHPERS_H

#include <stack>

#include "Types.h"
#include "NonlinearFunction.h"
#include "OpCode.h"

namespace Minotaur {

class CGraph;
class CNode;
typedef boost::shared_ptr<CGraph> CGraphPtr;
typedef std::deque<CNode *> CNodeQ;
typedef std::vector<CNode *> CNodeVector;
typedef std::map<ConstVariablePtr, CNode*, CompareVariablePtr> VarNodeMap;

class CGraphPers : public NonlinearFunction {
public:
  /// Default constructor.
  CGraphPers();

  /// Default constructor.
  ~CGraphPers();

  // base class method.
  NonlinearFunctionPtr clone(Int *err) const;

  // base class method.
  NonlinearFunctionPtr cloneWithVars(VariableConstIterator, Int *) const;

  // base class method.
  NonlinearFunctionPtr getPersp(VariablePtr z , int *err) const;

  // base class method.
  void computeBounds(Double *lb, Double *ub, Int *error);

  // Evaluate at a given array.
  Double eval(const Double *x, Int *err);

  // Evaluate gradient at a given array.
  void evalGradient(const Double *x, Double *grad_f, Int *error);

  // Evaluate hessian of at a given vector.
  void evalHessian(Double mult, const Double *x, 
                   const LTHessStor *stor, Double *values, 
                   Int *error);

  // Fill hessian sparsity.
  void fillHessStor(LTHessStor *stor);

  // Finalize hessian offsets, if needed.
  void finalHessStor(const LTHessStor *stor);

  // Add gradient values to sparse Jacobian
  void fillJac(const Double *x, Double *values, Int *error);

  /**
   * After adding all the nodes of the graph, finalize is called to create the
   * forward and backward traversal queues, and related book-keeping.
   */
  void finalize(); 

  // base class method.
  Double getFixVarOffset(VariablePtr v, Double val);

  // get type of function
  FunctionType getType() const;

  UInt getNumNodes();

  UInt getHessNz();

  /// Get the output node.
  const CNode* getOut() const;

  VariablePtr getVar(const CNode *cnode) const;

  // Get variables that exist in this function.
  void getVars(VariableSet *vars);

  Bool isIdenticalTo(CGraphPtr cg);

  // multiply by a constant.
  void multiply(Double c);

  /**
   * \brief Create a new node with one or two children, and add it to the
   * graph. The children should already be a nodes of the graph.
   *
   * \param [in] op OpCode of the new node
   * \param [in] lchild The left child of the new node. It should
   * never be NULL.
   * \param [in] rchild If the opcode needs two children, e.g. OpPlus, OpMult
   * etc, then rchild should be the right child. For univariate functions like
   * OpSqrt, it should be NULL.
   * \return The new node created in this function.
   */
  CNode* newNode(OpCode op, CNode *lchild, CNode *rchild);

  /**
   * \brief Create a new node with more than two children, and add it to the
   * graph. The children should already be a nodes of the graph.
   *
   * \param [in] op OpCode of the new node
   * \param [in] child An array of pointers to children. It should be size 'n'.
   * \param [in] n The size of array 'child'.
   * \return The new node created in this function.
   */
  CNode* newNode(OpCode op, CNode **child, UInt n);

  /**
   * \brief Create a new node with constant real value. It does not have any
   * children.
   *
   * \param [in] d The value.
   * \return The new node created in this function. Its opcode is OpNum
   */
  CNode* newNode(Double d);

  /**
   * \brief Create a new node with constant integer value. It does not have any
   * children.
   *
   * \param [in] i The value.
   * \return The new node created in this function. Its opcode is OpInt
   */
  CNode* newNode(Int i);

  /**
   * \brief Create a new node denoting an input variable. It does not have any
   * children. 
   *
   * The function checks if the variable already exists in the graph. If so,
   * it returns the pointer to that node. Otherwise it creates a new node.
   *
   * \param [in] v The variable.
   * \return The new node found or created in this function. Its opcode is
   * OpVar
   */
  CNode* newNode(VariablePtr v);

  // base class method.
  void prepJac(VarSetConstIter vbeg, VarSetConstIter vend);

  // base class method.
  void removeVar(VariablePtr v, Double val);

  /**
   * \brief Set the node that should be the output of this graph. This node
   * should already be a part of this graph (created by newNode() function).
   *
   * \param [in] node The node that is the output.
   */
  void setOut(CNode *node);

  // base class method.
  void subst(VariablePtr out, VariablePtr in, Double rat);

  // base class method.
  void varBoundMods(Double lb, Double ub, VarBoundModVector &mods,
                    SolveStatus *status);

  // display.
  void write(std::ostream &out) const;

private:
  /// The computational graph of the perspective function.
  CGraphPtr cgraph_; 

  /// The variable used to take the perspective
  VariablePtr z_;

  /// Tolerance for checking whether variable z is zero
  double zTol_;
};
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
