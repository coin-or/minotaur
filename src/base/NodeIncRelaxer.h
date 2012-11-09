// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2012 The MINOTAUR Team.
// 

/**
 * \file NodeIncRelaxer.h
 * \brief Declare the NodeIncRelaxer class. It creates relaxation by 
 * only incrementing changes made in ancestor nodes.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURNODEINCRELAXER_H
#define MINOTAURNODEINCRELAXER_H

#include "Handler.h"
#include "NodeRelaxer.h"

namespace Minotaur {

/**
 * The root relaxation is stored as rel_. In each node, we apply all
 * modifications stored in each ancestor of the node. When we are done
 * processing the node, we undo all these changes. 
 *
 * If we dive after processing a node, we do not need to undo all changes
 * and apply them again. We just apply the modifications of the parent.
 */
class NodeIncRelaxer : public NodeRelaxer {
public:
  /// Default constructor
  NodeIncRelaxer(EnvPtr env, HandlerVector handlers);

  /// Destroy
  ~NodeIncRelaxer();

  // Implement NodeRelaxer::CreateRootRelaxation()
  RelaxationPtr createRootRelaxation(NodePtr rootNode, Bool &prune);

  // Implement NodeRelaxer::CreateNodeRelaxation()
  RelaxationPtr createNodeRelaxation(NodePtr node, Bool dived, Bool &prune);

  // Implement NodeRelaxer::reset()
  void reset(NodePtr node, Bool diving);

  /**
   * Set the engine that is used to solve the relaxations. We need to set
   * it in order to be able to load warm-starts at a node.
   */
  void setEngine(EnginePtr e);

  // get the relaxation pointer, rel_.
  RelaxationPtr getRelaxation();

  /// Set your own relaxation pointer.
  void setRelaxation(RelaxationPtr rel);

private:
  /// Pointer engine used to solve the relaxation.
  EnginePtr engine_;

  /// Environment
  EnvPtr env_;

  /// Vector of handlers that will make the relaxation.
  HandlerVector handlers_;

  /**
   * True if only relaxation is modified, false if both problem and
   * relaxation are modified.
   */
  Bool modRelOnly_;

  /// The problem being solved by branch-and-bound.
  ProblemPtr p_;

  /**
   * We only keep one relaxation. It is modified at each node and then
   * reset.
   */
  RelaxationPtr rel_;
};

typedef boost::shared_ptr <NodeIncRelaxer> NodeIncRelaxerPtr;
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
