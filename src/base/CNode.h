//
//    Minotaur -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2025 The Minotaur Team.
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

  class CNode;

  struct CompareCNodes
  {
    bool operator()(const CNode *n1, const CNode *n2) const;
  };
  struct CompareCNodesR
  {
    bool operator()(const CNode *n1, const CNode *n2) const;
  };
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
   * op-code, children, parents and other auxiliary information to evaluate
   * the function and its derivative, check its properties etc.
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
    CNode(OpCode op, CNode **children, size_t num_child);

    /**
     * \brief: Construct a node from two children, left and right, and an
     * opcode
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
     * \param [in] out Pointer to the existing child node that will be
     * replaced
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
     * \param [nmap] in A map of nodes in of current CGraph to the nodes of
     * new CGraph.
     */
    void copyParChild(CNode *out,
                      std::map<const CNode *, CNode *> *nmap) const;

    /**
     * \brief Evaluate the function at this node.
     *
     * \param [in] x The point at which the CGraph containing this CNode is
     * being evaluated. x is used only for nodes that have opcode OpVar. For
     * all other nodes, the values of the children are used in evaluating the
     * function.
     * \param [out] error Nonzero if some error occurs in evaluation e.g. log
     * of a negative number, etc.
     */
    void eval(const double *x, int *error);

    /**
     * \brief Evaluate the value of just this node based on the single value
     * given as an input parameter. Ignores the values at the children nodes.
     * Used by Transformer classes. Applicable only to nodes that require a
     * single input (like OpUMinus, OpSqrt, etc) and not others (like
     * OpSumList, OpPlus, OpMult, etc.)
     *
     * \param [in] x The input argument for the function.
     * \param [out] error Nonzero if there is an error in evaluation like some
     * IEEE exception.
     * \return The value of the function evaluated.
     */
    double evalSingle(double x, int *error) const;

    /**
     * \brief Find the type of the function (constant, linear, polynomial,
     * etc.) based on the function type of children.
     */
    FunctionType findFType();

    /**
     * \brief Calculate the derivative of the function at this node based on
     * values and gradients of children nodes. Used in forward-mode gradient
     * calculation.
     */
    void fwdGrad();

    /// \return The value of the boolean flag.
    bool getB() const { return b_; };

    /// \return The value of the gradient value.
    double getG() const { return g_; };

    /// \return The value of the hessian value at this node.
    double getH() const { return h_; };

    /// \return The ID
    size_t getId() const { return id_; };

    /// \return index of the node
    size_t getIndex() const { return index_; };

    /// \return The pointer to left child
    CNode *getL() const { return l_; };

    /// \return The lower bound
    double getLb() const { return lb_; };

    /// \return Pointer to the beginning of array of pointers to children
    CNode **getListL() const { return child_; };

    /// \return Pointer to the end of array of pointers to children
    CNode **getListR() const { return child_ + numChild_; };

    /// \return The opcode of this node.
    OpCode getOp() const { return op_; };
    CQIter2 *getParB() const { return parB_; };

    /// \return The pointer to right child
    CNode *getR() const { return r_; };
    int getTempI() const { return ti_; };

    /// \return The stored function type (does not evaluate).
    FunctionType getType() const { return fType_; };

    /// \return The upper bound
    double getUb() const { return ub_; };

    /// \return The unique parent if any, otherwise return NULL
    CNode *getUPar() const { return uPar_; };

    /// \return Pointer to the variable if the node is OpVar, otherwise NULL
    const Variable *getV() const { return v_; };

    /// \return The value of function stored at this node (does not evaluate)
    double getVal() const { return val_; };

    /**
     * \brief Push gradient values to children for reverse mode gradient
     * evaluation.
     *
     * \param [out] error Nonzero if there is an error in evaluation.
     */
    void grad(int *error);

    /// \return The number of children nodes.
    size_t numChild() const { return numChild_; };

    /// \return The number of parents
    size_t numPar() const { return numPar_; };

    /**
     * \brief Find whether child-nodes must participate in evaluation  of the
     * current column of the hessian-matrix. Sets values of internal flags and
     * variables.
     */
    void propHessSpa();

    /// A variant of propHessSpa()
    void propHessSpa2(CNodeRSet *nset);

    /**
     * \brief Deduce and push new bounds to child-nodes based on bounds of
     * this node.
     *
     * \param [out] is_inf True if it detects inconsistency, i.e.
     * infeasibility
     * \param [out] error Nonzero if some error is encountered in evaluation.
     */
    void propBounds(bool *is_inf, int *error);

    /**
     * \brief Reverse mode hessian evaluation. Hessian values are pushed to
     * children nodes.
     *
     * \param [out] error Nonzero if some error is encountered in evaluation.
     */
    void hess(int *error);

    /**
     * \brief Reverse mode hessian evaluation with sparsity detection. Hessian
     * values are pushed to children nodes.
     *
     * \param [in, out] nset Children are added to nset if they participate in
     * Hessian.
     * \param [out] error Nonzero if some error is encountered in evaluation.
     */
    void hess2(CNodeRSet *nset, int *error);

    /**
     * \brief Set the boolean flag.
     *
     * \param [in] b The flag.
     */
    void setB(bool b) { b_ = b; };

    /**
     * \brief Set lower and upper bounds on the node.
     *
     * \param [in] lb Lower bound.
     * \param [in] ub Upper bound.
     */
    void setBounds(double lb, double ub)
    {
      lb_ = lb;
      ub_ = ub;
    };

    /**
     * \brief Set the constant double value (OpNum and OpInt).
     *
     * \param [in] d The value.
     */
    void setDouble(double d) { d_ = d; };

    /**
     * \brief Set the value of the derivative (reverse mode gradient)
     *
     * \param [in] g The value.
     */
    void setG(double g) { g_ = g; };

    /**
     * \brief Set the value of the derivative (forward mode gradient)
     *
     * \param [in] gi The value.
     */
    void setGi(double gi) { gi_ = gi; };

    /**
     * \brief Set the value of the hessian.
     *
     * \param [in] h The value.
     */
    void setH(double h) { h_ = h; };

    /**
     * \brief Set the value of the node ID.
     *
     * \param [in] i The value.
     */
    void setId(size_t i) { id_ = i; };

    /**
     * \brief Set the value of the node index.
     *
     * \param [in] i The value.
     */
    void setIndex(size_t i) { index_ = i; };


    /**
     * \brief Set the pointer to the left-most child
     *
     * \param [in] n The pointer.
     */
    void setL(CNode *n) { l_ = n; };

    /**
     * \brief Set the OpCode.
     *
     * \param [in] op The OpCode value.
     */
    void setOp(OpCode op) { op_ = op; };

    /**
     * \brief Set the pointer to the right-most child
     *
     * \param [in] n The pointer.
     */
    void setR(CNode *n) { r_ = n; };

    /**
     * \brief Set the value of the temporary integer ti_
     *
     * \param [in] ti The pointer.
     */
    void setTempI(int i) { ti_ = i; };

    /**
     * \brief Set the value of function type of the graph rooted at this node
     *
     * \param [in] t The type.
     */
    void setType(FunctionType t);

    /**
     * \brief Set the value of function at this node
     *
     * \param [in] v The value.
     */
    void setVal(double v);

    /**
     * \brief Set the Variable at this node (for OpVar nodes)
     *
     * \param [in] v The variable pointer.
     */

    void setV(VariablePtr v)
    {
      v_ = v;
    };  // Changed (showing error while compiling)

    /**
     * \brief Find bounds deduced from the bounds of children nodes and update
     * lb_ and ub_
     *
     * \param [out] error Nonzero if some error is encountered in evaluating
     * the bounds.
     */
    void updateBnd(int *error);

    /// Print the function expression at current node only.
    void write(std::ostream &out) const;

    /// Print the function expression at current node and the sub-tree.
    void writeSubExp(std::ostream &out) const;

    /**
     * \brief Write the function expression in ASL's text format (reverse
     * polish notation) for the current node and the sub-tree.
     *
     * \param [in] s stream to which the expression is appended
     * \param [out] err Nonzero if some error is encountered
     */
    void writeSubNl(std::stringstream &s, int *err) const;

  protected:
    bool b_;  /// Boolean flag used in finding hessian sparsity
    CNode *
        *child_;  /// array of size numChild_ + 1. The last pointer is a null
    double d_;    /// Constant double value in OpNum and OpInt nodes
    FunctionType fType_;  /// Function type of the graph rooted at this node
    double g_;   /// Value of the derivative of output w.r.t. this node
                 /// (reverse mode)
    double gi_;  /// Value of the derivative of this node w.r.t. to a given
                 /// var (forward mode)
    double h_;   /// Value of the hessian
    size_t id_;  /// Unique ID of a dependent node. 0 for independent nodes.
    size_t index_;     /// Unique index of the node
    CNode *l_;         /// Left child
    double lb_;        /// lower bound that a node can achieve
    size_t numChild_;  /// Number of children
    size_t numPar_;    /// Number of parents
    OpCode op_;        /// Operation code
    CQIter2 *parB_;    /// Pointer to the first parent (parents begin)
    CQIter2 *parE_;    /// Pointer to the last parent (parents end)
    CNode *r_;         /// Right child
    int ti_;           /// Temporary integer variable to find patterns of
                       /// sparsity in the hessian.
    double ub_;        /// upper bound that a node can achieve


    CNode *uPar_;        /// Unique parent of this node. NULL if the node has
                         /// multiple parents
    const Variable *v_;  /// Pointer the variable in case it is OpVar
    double val_;  /// Current value of the expression based on the value
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
  typedef std::vector<CNode *> CNodeVector;
}  //namespace Minotaur


#endif
