// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2013 The MINOTAUR Team.
// 

/**
 * \file NodeIncRelaxer.cpp
 * \brief Define the class NodeIncRelaxer.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <iostream>
#include <stack>

#include "MinotaurConfig.h"
#include "Engine.h"
#include "Environment.h"
#include "Node.h"
#include "NodeIncRelaxer.h"
#include "Option.h"
#include "Relaxation.h"


using namespace Minotaur;


NodeIncRelaxer::NodeIncRelaxer (EnvPtr env, HandlerVector handlers) 
  : engine_(EnginePtr()),  // NULL
    env_(env),
    handlers_(handlers),
    rel_(RelaxationPtr()) // NULL
{
  modRelOnly_ = env_->getOptions()->findBool("modify_rel_only")->getValue();
}


NodeIncRelaxer::~NodeIncRelaxer ()
{
  rel_.reset();
  handlers_.clear();
  env_.reset();
  engine_.reset();
}


RelaxationPtr NodeIncRelaxer::createRootRelaxation(NodePtr, Bool &prune)
{
  prune = false;
  rel_ = (RelaxationPtr) new Relaxation();
  for (HandlerIterator h = handlers_.begin(); h != handlers_.end() && !prune; 
      ++h) {
    (*h)->relaxInitInc(rel_, &prune);
    if (prune) {
      break;
    }
  }
  if (!prune) {
    rel_->calculateSize();
  }

  if (engine_) {
    engine_->load(rel_);
  }

  return rel_;
}


// Set and load the engine that is used to solve the relaxation.
void NodeIncRelaxer::setEngine(EnginePtr e)
{
  engine_ = e;
  if (rel_) {
    engine_->load(rel_);
  }
}


RelaxationPtr NodeIncRelaxer::createNodeRelaxation(NodePtr node, Bool dived, 
                                                   Bool &prune)
{
  NodePtr t_node; // temporary
  WarmStartPtr ws;
  prune = false;

  if (!dived) {
    // traceback to root and put in all modifications that need to go into the
    // relaxation and the engine.
    std::stack<NodePtr> predecessors;
    t_node = node->getParent();

    while (t_node) {
      predecessors.push(t_node);
      t_node = t_node->getParent();
    }

    // starting from the top, put in modifications made at each node to the
    // engine
    if (modRelOnly_) {
      while (!predecessors.empty()) {
        t_node = predecessors.top();
        t_node->applyMods(rel_);
        predecessors.pop();
      }
    } else {
      while (!predecessors.empty()) {
        t_node = predecessors.top();
        t_node->applyMods(rel_, p_);
        predecessors.pop();
      }
    }
  } 

  // put in the modifications that were used to create this node from
  // its parent.
  if (modRelOnly_) {
    node->applyMods(rel_);
  } else {
    node->applyMods(rel_, p_);
  }

  for (HandlerIterator h = handlers_.begin(); h != handlers_.end() && !prune; 
      ++h) {
    (*h)->relaxNodeInc(node, rel_, &prune);
    if (prune) {
      break;
    }
  }

  // give the warm start info to the engine.
  if (!prune) {
    ws = node->getWarmStart();
    if (ws) {
      engine_->loadFromWarmStart(ws);
    }
  } else if (ws) {
    node->removeWarmStart();
  }
  return rel_;
}


void NodeIncRelaxer::reset(NodePtr node, Bool diving)
{
  NodePtr t_node = node;

  if (!diving) {
    if (modRelOnly_) {
      while (t_node) {
        t_node->undoMods(rel_);
        t_node = t_node->getParent();
      }
    } else {
      while (t_node) {
        t_node->undoMods(rel_, p_);
        t_node = t_node->getParent();
      }
    }
  }
}


RelaxationPtr NodeIncRelaxer::getRelaxation()
{
  return rel_;
}


void NodeIncRelaxer::setRelaxation(RelaxationPtr rel)
{
  rel_ = rel;
}


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
