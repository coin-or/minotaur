//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
//


/**
 * \file CNode.h
 * \brief Declare class CNode to represent node of a computational graph of a
 * nonlinear function.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURCNODE_H
#define MINOTAURCNODE_H

#include "OpCode.h"
#include "Types.h"

namespace Minotaur {

class Variable;
class CNode;

struct CompareCNodes {
  bool operator()(const CNode* n1, const CNode *n2) const;
};
struct CompareCNodesR {
  bool operator()(const CNode* n1, const CNode *n2) const;
};
typedef boost::shared_ptr<Variable> VariablePtr;
typedef std::set<CNode *, CompareCNodes> CNodeSet;
typedef std::set<CNode *, CompareCNodesR> CNodeRSet;

class CQIter2 {
public:
  CNode *node;
  CQIter2 *next;
  CQIter2 *prev;
};

/**
 * \brief CNode denotes a node in the computational graph. It stores the
 * op-code, children, parents and other auxiliary information to evaluate the
 * function and its derivative, check its properties etc.
 */
class CNode {
public:

  /// Default constructor
  CNode();

  /**
   * \brief: Construct a node from its children and an opcode
   *
   * Constructor with a specific opcode and children. 
   * \param [in] op Opcode of the new CNode
   * \param [in] children Pointer to an array of child nodes, can be null
   * \param [in] Number of children, can be zero or more
   */
  CNode(OpCode op, CNode **children, UInt num_child);

  /**
   * \brief: Construct a node from two children, left and right, and an opcode
   *
   * Constructor with a specific opcode and two children. 
   * \param [in] op Opcode of the new CNode
   * \param [in] lchild Pointer to the left child node
   * \param [in] rchild Pointer to the right child node
   */
  CNode(OpCode op, CNode *lchild, CNode *rchild);

  /// Destroy.
  ~CNode();

  /// Clone the node. Does not copy pointers to parents, children, etc.
  CNode *clone() const;

  /**
   * \brief Add a parent to a given node (a parent depends on the child in
   * function evaluation).
   *
   * \parm [in] Pointer to the parent node
   */
  void addPar(CNode *node);

  /**
   * \brief Replace one of the children of a given node by another node.
   *
   * \param [in] out Pointer to the existing child node that will be replaced
   * \param [in] in  Pointer to the new child node that will replace the
   * existing child.
   */
  void changeChild(CNode *out, CNode *in);

  /**
   * \brief Make copies of links pointing to parents and children to another
   * node. Used in cloning the CGraph.
   *
   * \param [in,out] out The node to which the links must be copied. This is
   * the clone.
   * \param [nmap] in A map of nodes in of current CGraph to the nodes of new
   * CGraph.
   */
  void copyParChild(CNode *out, std::map<const CNode*, CNode*> *nmap) const;

  /**
   * \brief Evaluate the function at this node.
   *
   * \param [in] x The point at which the CGraph containing this CNode is
   * being evaluated. x is used only for nodes that have opcode OpVar. For all
   * other nodes, the values of the children are used in evaluating the
   * function.
   * \param [out] error Nonzero if some error occurs in evaluation e.g. log of
   * a negative number, etc.
   */
  void eval(const double *x, int *error);

  /**
   * \brief TODO: Unused.
   */
  double eval(double x, int *error) const;

  FunctionType findFType();
  void fwdGrad();
  bool getB() const { return b_; };
  double getG() const { return g_; };
  double getH() const {return h_;};
  UInt getId() const {return id_;};
  CNode* getL() const { return l_; };
  double getLb() const { return lb_; };
  CNode** getListL() const { return child_; };
  CNode** getListR() const { return child_+numChild_; };
  OpCode getOp() const { return op_; };
  CQIter2* getParB() const { return parB_; };
  CNode* getR() const { return r_; };
  int getTempI() const { return ti_; };
  FunctionType getType() const { return fType_; } ;
  double getUb() const { return ub_; };
  CNode* getUPar() const { return uPar_; };
  const Variable* getV() const { return v_; };
  double getVal() const { return val_; };
  void grad(int *error);
  UInt numChild() const;
  UInt numPar() const { return numPar_; };
  void propHessSpa();
  void propHessSpa2(CNodeRSet *nset);
  void propBounds(bool *is_inf, int *error);
  void hess(int *error);
  void hess2(CNodeRSet *nset, int *error);
  void setB(bool b) {b_ = b;};
  void setBounds(double lb, double ub) {lb_ = lb; ub_ = ub; };
  void setDouble(double d) {d_ = d;};
  void setG(double g) {g_ = g;};
  void setGi(double gi) {gi_ = gi;};
  void setH(double h) {h_ = h;};
  void setId(UInt i) {id_ = i;};
  void setInt(int i) {i_ = i;};
  void setL(CNode *n) {l_ = n;};
  void setOp(OpCode op) {op_ = op;};
  void setR(CNode *n) {r_ = n;};
  void setTempI(int i) { ti_ = i; };
  void setType(FunctionType t);
  void setVal(double v);
  void setV(VariablePtr v) {v_ = v.get();};
  void updateBnd(int *error);

  void write(std::ostream &out) const;
  void writeSubExp(std::ostream &out) const;

protected:
  bool b_;        /// Boolean flag used in finding hessian sparsity
  CNode **child_; /// array of size numChild_ + 1. The last pointer is a null
  double d_;      /// Constant double value in OpNum and OpInt nodes
  FunctionType fType_; /// Function type of the graph rooted at this node
  double g_;      /// Value of the derivative of output w.r.t. this node
                  /// (reverse mode)
  double gi_;     /// Value of the derivative of this node w.r.t. to a given var
                  /// (forward mode)
  double h_;      /// Value of the hessian
  int i_;         /// TODO: Is it unused?
  UInt id_;       /// Unique ID of the node, used in the map
  CNode *l_;      /// Left child
  double lb_;     /// lower bound that a node can achieve
  UInt numChild_; /// Number of children
  UInt numPar_;   /// Number of parents
  OpCode op_;     /// Operation code
  CQIter2 *parB_; /// Pointer to the first parent (parents begin)
  CQIter2 *parE_; /// Pointer to the last parent (parents end)
  CNode *r_;      /// Right child
  int ti_;        /// Temporary integer variable to find patterns of
                  /// sparsity in the hessian.
  double ub_;     /// upper bound that a node can achieve

  
  CNode *uPar_;   /// Unique parent of this node. NULL if the node has
                  /// multiple parents
  const Variable* v_; /// Pointer the variable in case it is OpVar
  double val_;    /// Current value of the expression based on the value
                  /// of the children

  /**
   * \brief Change the current bounds of the function value at this node to
   * new bounds only if the new bounds are tighter. Also checks whether the
   * new bounds are inconsistent.
   *
   * \param [in] lb New lower bound
   * \param [in] ub New upper bound
   * \param [out] is_inf Set to true if lb is more than ub after the update.
   * Otherwise, remains unchanged.
   */
  void propBounds_(double lb, double ub, bool *is_inf);

};
typedef std::vector<CNode*> CNodeVector;
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
