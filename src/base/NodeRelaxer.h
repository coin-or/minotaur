// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file NodeRelaxer.h
 * \brief Define the NodeRelaxer class for creating relaxation for nodes in the
 *  branch-and-bound algorithm.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURNODERELAXER_H
#define MINOTAURNODERELAXER_H

#include "Types.h"
#include "Modification.h"
#include "Relaxation.h"
#include "SolutionPool.h"

namespace Minotaur {

/**
 * NodeRelaxer class is used to create relaxations for nodes in a tree.
 * The relaxation at a node can be created in different ways, depending upon
 * the problem and the algorithm used. For instance, in NLP-BB, we can
 * simply relax the integrality constraints. In this case, we can just keep
 * one copy of the relaxation and change bounds in different nodes. For LP
 * based branch-and-reduce, the relaxation could be very different in a
 * node, and we may want to create new relaxation in every node.
 * 
 * Relaxations are created using handlers. Therefore, the NodeRelaxer must
 * have compatible handlers. For instance, we can not have a handler return
 * cuts in NLP-BB. Also, we can not have handler branch on a new variable
 * created in a relaxation, if we are creating in each node, new relaxations
 * from scratch.
 * 
 * This is the abstract base class.
 */
class NodeRelaxer {
public:
  /// Default constructor.
  NodeRelaxer() { }

  /// Destroy.
  virtual ~NodeRelaxer() { }

  /**
   * \brief Create the root node relaxation.
   * \param[in] rootNode A pointer to root node.
   * \param[out] prune is true if the root node can be pruned, either because
   * the relaxation is found to be infeasible or because a trivial solution
   * has been found.
   * \returns a pointer to the relaxation. The method must create a relaxation
   * even if we want to prune the root node.
   */
  virtual RelaxationPtr createRootRelaxation(NodePtr rootNode,
                                             SolutionPool *sp, bool &prune) =0; 

  /**
   * Create a relaxation for the current node. Relaxation can be an
   * incremental change to an existing relaxation or it can be a new
   * relaxation created from scratch.
   * \param[in] node the node for which relaxation is created
   * \param[in] dived is true if we dived down from the previous node, i.e.
   * if this node is processed right after its parent.
   * \param[out] prune is true if the node was found to be infeasible after
   * creating the relaxation.
   */
  virtual RelaxationPtr createNodeRelaxation(NodePtr node, bool dived, 
                                             bool &prune) = 0;

  /**
   * After processing the node, some node relaxers may like to make
   * changes. This function is the place to do it. diving is true if the
   * next node to be processed is a child node of the current node.
   */
  virtual void reset(NodePtr node, bool diving) = 0;

  /**
   * Return a pointer to the last relaxation that was created by this
   * relaxer.
   */
  virtual RelaxationPtr getRelaxation() = 0;
};

typedef NodeRelaxer* NodeRelaxerPtr;
}
#endif

