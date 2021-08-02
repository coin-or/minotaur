// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
// 

/**
 * \file TreeManager.cpp
 * \brief Define class TreeManager for managing tree for Branch-and-Bound.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "Environment.h"
#include "Node.h"
#include "NodeHeap.h"
#include "NodeStack.h"
#include "Operations.h"
#include "Option.h"
#include "Timer.h"
#include "TreeManager.h"

using namespace Minotaur;
    
    
TreeManager::TreeManager(EnvPtr env) 
: bestLowerBound_(-INFINITY),
  bestUpperBound_(INFINITY),
  cutOff_(INFINITY),
  doVbc_(false),
  etol_(1e-6),
  size_(0),
  timer_(0)
{
  std::string s = env->getOptions()->findString("tree_search")->getValue();
  if ("dfs"==s) {
    searchType_ = DepthFirst;
  } else if ("bfs"==s) {
    searchType_ = BestFirst;
  } else if ("BthenD"==s) {
    searchType_ = BestThenDive;
  } else {
     assert (!"search strategy must be defined!");
  }

  switch (searchType_) {
   case (DepthFirst):
     activeNodes_ = (NodeStackPtr) new NodeStack();
     break;
   case (BestFirst):
   case (BestThenDive):
     activeNodes_ = (NodeHeapPtr) new NodeHeap(NodeHeap::Value);
     break;
   default:
     assert (!"search strategy must be defined!");
  }

  aNode_ = NodePtr();
  cutOff_ = env->getOptions()->findDouble("obj_cut_off")->getValue();
  s = env->getOptions()->findString("vbc_file")->getValue();
  if (s!="") {
    vbcFile_.open(s.c_str());
    if (!vbcFile_.is_open()) {
      std::cerr << "cannot open file " << s << " for writing tree information.";
    } else {
      doVbc_ = true;
      vbcFile_ << "#TYPE: COMPLETE TREE" << std::endl;
      vbcFile_ << "#TIME: SET" << std::endl;
      vbcFile_ << "#BOUNDS: NONE" << std::endl;
      vbcFile_ << "#INFORMATION: STANDARD" << std::endl;
      vbcFile_ << "#NODE_NUMBER: NONE" << std::endl;
      timer_ = env->getNewTimer();
      timer_->start();
    }
  }
} 


TreeManager::~TreeManager()
{
  clearAll();
  delete activeNodes_;
  if (doVbc_) {
    vbcFile_.close();
    delete timer_;
  }
}


bool TreeManager::anyActiveNodesLeft()
{
  return !activeNodes_->isEmpty();
}


NodePtr TreeManager::branch(Branches branches, NodePtr node, WarmStartPtr ws)
{
  BranchPtr branch_p;
  NodePtr new_cand = NodePtr(); // NULL
  NodePtr child;
  bool is_first = false;

  if (searchType_ == DepthFirst || searchType_ == BestThenDive) {
    is_first = true;
  }
  for (BranchConstIterator br_iter=branches->begin(); br_iter!=branches->end();
      ++br_iter) {
    branch_p = *br_iter;
    child = (NodePtr) new Node(node, branch_p);
    child->setLb(node->getLb());
    child->setTbScore(node->getTbScore());
    child->setDepth(node->getDepth()+1);
    node->addChild(child);
    if (is_first) {
      child->setWarmStart(ws);
      insertCandidate_(child, true);
      is_first = false;
      new_cand = child;
    } else {
      // We make a copy of the pointer to warm-start, not the full copy of the
      // warm-start.
      child->setWarmStart(ws);
      insertCandidate_(child);
    }
  }
  if (doVbc_) {
    vbcFile_ << toClockTime(timer_->query()) << " P " << node->getId()+1 << " "
             << VbcSolved << std::endl;
    if (new_cand) {
      vbcFile_ << toClockTime(timer_->query()) << " P "
               << new_cand->getId()+1 << " " << VbcSolving << std::endl;
    }
  }
  aNode_ = new_cand; // can be NULL
  return new_cand;
}


void TreeManager::clearAll()
{
  NodePtr n;
  NodePtrIterator node_i;

  if (aNode_) {
    removeNodeAndUp_(aNode_);
  }
  while (false==activeNodes_->isEmpty()) {
    n = activeNodes_->top();
    removeNodeAndUp_(n);
    activeNodes_->pop();
  }
}


UInt TreeManager::getActiveNodes() const
{
  return activeNodes_->getSize();
}


NodePtr TreeManager::getCandidate()
{
  NodePtr node = NodePtr(); // NULL
  //aNode_.reset();
  aNode_ = 0;
  while (activeNodes_->getSize() > 0) {
    node = activeNodes_->top();
    if (shouldPrune_(node)) {
      removeActiveNode(node);
      pruneNode(node);
      //node.reset(); // NULL
      node = 0;
    } else {
      if (doVbc_) {
        vbcFile_ << toClockTime(timer_->query()) << " P " << node->getId()+1
                 << " " << VbcSolving << std::endl;
      }
      break;
    }
  } 
  return node; // can be NULL
  // do not pop the head until the candidate has been processed.
}


double TreeManager::getCutOff()
{
  return cutOff_;
}


double TreeManager::getPerGap()
{
  // for minimization problems, gap = (ub - lb)/(ub) * 100
  // so that if one has a ub, she can say that the solution can not be more
  // than gap% away from the current ub.
  double gap = 0.0;
  if (bestUpperBound_ >= INFINITY) {
    gap = INFINITY;
  } else if ((bestUpperBound_ > etol_) && (fabs(bestLowerBound_) < etol_)) {
    gap = 100.0;
  } else {
    gap = (bestUpperBound_ - bestLowerBound_)/(fabs(bestUpperBound_)+etol_) 
      * 100.0;
    if (gap<0.0) {
      gap = 0.0;
    }
  }
  return gap;
}


double TreeManager::getLb()
{
  return bestLowerBound_;
}


UInt TreeManager::getSize() const
{
  return size_;
}


double TreeManager::getUb()
{
  return bestUpperBound_;
}


void TreeManager::insertCandidate_(NodePtr node, bool pop_now)
{
  assert(size_>0);

  // set node id and depth
  node->setId(size_);
  node->setDepth(node->getParent()->getDepth()+1);

  ++size_;

  // add node to the heap/stack of active nodes. If pop_now is true, the node
  // is processed right after creating it; we don't
  // want to keep it in activeNodes (e.g. while diving)
  if (!pop_now) {
    activeNodes_->push(node);
  } 
  if (doVbc_) {
    vbcFile_ << toClockTime(timer_->query()) << " N "
      << node->getParent()->getId()+1 << " " << node->getId()+1
      << " " << VbcActive << std::endl;
  }
}


void TreeManager::insertRoot(NodePtr node)
{
  assert(size_==0);
  assert(activeNodes_->getSize()==0);

  node->setId(0);
  node->setDepth(0);
  activeNodes_->push(node);
  ++size_;
  if (doVbc_) {
    // father node color
    vbcFile_ << toClockTime(timer_->query()) << " N 0 1 " << VbcSolving
             << std::endl; 
  }
}


void TreeManager::pruneNode(NodePtr node)
{
  // XXX: if required do something before deleting the node.
  removeNodeAndUp_(node);
}


void TreeManager::removeActiveNode(NodePtr node)
{
  if (doVbc_) {
    if (node->getStatus()==NodeOptimal) {
      vbcFile_ << toClockTime(timer_->query()) << " P "
        << activeNodes_->top()->getId()+1 << " " << VbcFeas << std::endl;
    } else if (node->getStatus()!=NodeInfeasible && node->getStatus()!=NodeHitUb) {
      vbcFile_ << toClockTime(timer_->query()) << " P "
               << activeNodes_->top()->getId()+1 << " " << VbcSolved << std::endl;
    } 
  }

  activeNodes_->pop();
  // dont remove the head until the candidate has been processed.
}


void TreeManager::removeNode_(NodePtr node) 
{
  NodePtr cNode = 0;
  NodePtr  parent = node->getParent();;
  NodePtrIterator node_i;

  if (node->getId()>0) {
    // Find the iterator corresponding to this node
    for (node_i = parent->childrenBegin(); node_i != parent->childrenEnd(); 
        ++node_i) {
      cNode = *node_i;
      if (cNode == node) {
        break;
      }
    }

    if (cNode==node) {
      parent->removeChild(node_i);
      node->removeParent();
      if (doVbc_) {
        VbcColors c;
        if (node->getStatus()==NodeHitUb) {
          c = VbcSubOpt;
        } else if (node->getStatus()==NodeInfeasible) {
          c = VbcInf;
        } else if (node->getStatus()==NodeOptimal) {
          c = VbcFeas;
        } else {
          c = VbcSubInf;
        }
        vbcFile_ << toClockTime(timer_->query()) << " P " << node->getId()+1 << " " 
                 << c << std::endl;
      }
    } else {
      assert (!"Current node is not in its parent's list of children!");
    }
  } 
  delete node;
}


void TreeManager::removeNodeAndUp_(NodePtr node) 
{
  NodePtr parent = node->getParent();

  // remove the given node
  removeNode_(node);

  // remove the ancestors of the given node, if they have no children left
  while (parent && parent->getNumChildren()==0) {
    node = parent;
    parent = node->getParent();
    removeNode_(node);
  }
}


void TreeManager::setCutOff(double value)
{
  cutOff_ = value;
}


void TreeManager::setUb(double value)
{
  bestUpperBound_ = value;
  if (value < cutOff_) {
    cutOff_ = value;
  }
}


bool TreeManager::shouldDive()
{
  if (searchType_ == DepthFirst || searchType_ == BestThenDive) {
    return true;
  } 
  return false;
}


bool TreeManager::shouldPrune_(NodePtr node)
{
  double lb = node->getLb();
  if (lb > cutOff_ - etol_ || 
      fabs(bestUpperBound_-lb)/(fabs(bestUpperBound_)+etol_)*100 < etol_) {
    node->setStatus(NodeHitUb);
    return true;
  }
  return false;
}


double TreeManager::updateLb()
{
  // this could be an expensive operation. Try to avoid it.
  bestLowerBound_ = activeNodes_->getBestLB();

  return bestLowerBound_;
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
