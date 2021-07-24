// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
// 

/**
 * \file Node.cpp
 * \brief Define class Node for storing information about nodes.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Branch.h"
#include "Constraint.h"
#include "Cut.h"
#include "Function.h"
#include "LinearFunction.h"
#include "Modification.h"
#include "Node.h"
#include "Relaxation.h"
#include "WarmStart.h"

using namespace Minotaur;
using namespace std;

Node::Node()
  : branch_(0),
    depth_(0),
    id_(0),
    lb_(-INFINITY),
    pMods_(0), 
    rMods_(0), 
    parent_(NodePtr()),
    status_(NodeNotProcessed),
    vioVal_(0),
    tbScore_(0),
    cutPool_(0),
    ws_(0)
{
}


Node::Node(NodePtr parentNode, BranchPtr branch)
  : branch_(branch),
    depth_(0),
    id_(0),
    pMods_(0), 
    rMods_(0), 
    parent_(parentNode),
    status_(NodeNotProcessed),
    vioVal_(0),
    tbScore_(0),
    cutPool_(0),
    ws_(0)
{
  lb_ = parentNode->getLb();
}


Node::~Node()
{
  removeWarmStart();
  if (branch_) {
    delete branch_;
  }
  for (ModificationConstIterator it=pMods_.begin(); it!=pMods_.end(); ++it) {
    delete *it;
  }
  for (ModificationConstIterator it=rMods_.begin(); it!=rMods_.end(); ++it) {
    delete *it;
  }
  for (CutListIter it=cutPool_.begin(); it!=cutPool_.end(); ++it) {
    delete *it;
  }

  pMods_.clear();
  rMods_.clear();
  cutPool_.clear();
  children_.clear();
}


void Node::addChild(NodePtr childNode)
{
  children_.push_back(childNode);
}


void Node::applyPMods(ProblemPtr p)
{
  ModificationConstIterator mod_iter;
  ModificationPtr mod;
  // first apply the mods that created this node from its parent
  if (branch_) {
    for (mod_iter=branch_->pModsBegin(); mod_iter!=branch_->pModsEnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod->applyToProblem(p);
    }
  }
  // now apply any other mods that were added while processing it.
  for (mod_iter=pMods_.begin(); mod_iter!=pMods_.end(); ++mod_iter) {
    mod = *mod_iter;
    mod->applyToProblem(p);
  }
}


void Node::applyRMods(RelaxationPtr rel)
{
  ModificationConstIterator mod_iter;
  ModificationPtr mod;
  // first apply the mods that created this node from its parent
  if (branch_) {
    for (mod_iter=branch_->rModsBegin(); mod_iter!=branch_->rModsEnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod->applyToProblem(rel);
    }
  }
  // now apply any other mods that were added while processing it.
  for (mod_iter=rMods_.begin(); mod_iter!=rMods_.end(); ++mod_iter) {
    mod = *mod_iter;
    mod->applyToProblem(rel);
  }
}


void Node::addCutToPool(CutPtr cut, RelaxationPtr rel)
{
  FunctionPtr f;
  VariablePtr v;
  LinearFunctionPtr lf, lfnew;
  lf = cut->getFunction()->getLinearFunction();
  if (lf) {
    lfnew = (LinearFunctionPtr) new LinearFunction();
    for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();++it) {
      v = it->first;
      //transform the cut
      lfnew->addTerm(rel->getRelaxationVar(rel->getOriginalVar(v)),it->second);
    }
    f = (FunctionPtr) new Function(lfnew);
    CutPtr c = (CutPtr) new Cut(rel->getNumVars(),f, cut->getLb(), cut->getUb(), false,false);
    //c->setCons(cut->getConstraint());
    c->setName_(cut->getName());
    cutPool_.push_back(c);
  }
}


void Node::applyCutsByIndex(RelaxationPtr rel)
{
  if (cutPool_.size() > 0) {
    FunctionPtr f;
    VariablePtr v;
    LinearFunctionPtr lf, lfnew;
    for (CutListIter it=cutPool_.begin(); it!=cutPool_.end(); ++it) {
      //lf = dynamic_cast <LinearFunction*> ((*it)->getFunction());
      lf = (*it)->getFunction()->getLinearFunction();
      if (lf) {
        lfnew = (LinearFunctionPtr) new LinearFunction();
        for (VariableGroupConstIterator it1=lf->termsBegin(); it1!=lf->termsEnd();++it1) {
          v = it1->first;
          //transform the cut
          lfnew->addTerm(rel->getRelaxationVar(rel->getOriginalVar(v)),it1->second);
        }
        f = (FunctionPtr) new Function(lfnew);
        rel->newConstraint(f, (*it)->getLb(), (*it)->getUb(), (*it)->getName());
      }
    }
  }
}


void Node::applyRModsTrans(RelaxationPtr rel)
{
  ModificationConstIterator mod_iter;
  ModificationPtr mod;
  ModificationPtr pmod1, mod2;
  ProblemPtr p=0;   //not used, just passed

  // first apply the mods that created this node from its parent
  if (branch_) {
    for (mod_iter=branch_->rModsBegin(); mod_iter!=branch_->rModsEnd();
        ++mod_iter) {
      mod = *mod_iter;
      //convert modifications applicable for other relaxation to this one
      pmod1 = mod->fromRel(rel, p);
      if (pmod1) {
        mod2 = pmod1->toRel(p, rel);
        mod2->applyToProblem(rel);
        delete mod2; mod2 = 0;
      } else {
        mod->applyToProblem(rel);
      }
      delete pmod1; pmod1 = 0;
    }
  }
  // now apply any other mods that were added while processing it.
  for (mod_iter=rMods_.begin(); mod_iter!=rMods_.end(); ++mod_iter) {
    mod = *mod_iter;
    pmod1 = mod->fromRel(rel, p);
    if (pmod1) {
      mod2 = pmod1->toRel(p, rel);
      mod2->applyToProblem(rel);
      delete mod2; mod2 = 0;
    } else {
      mod->applyToProblem(rel);
    }
    delete pmod1; pmod1 = 0;
  }
}


void Node::applyMods(RelaxationPtr rel, ProblemPtr p)
{
  applyPMods(p);
  applyRMods(rel);
}


void Node::removeChild(NodePtrIterator childNodeIter)
{
  children_.erase(childNodeIter);
}


void Node::removeChildren()
{
  children_.clear();
}


void Node::removeParent()
{
  parent_ = 0;
}


void Node::removeWarmStart()
{ 
  if (ws_) {
    assert(ws_->getUseCnt()>0);
    ws_->decrUseCnt();
    if (0==ws_->getUseCnt()) {
      delete ws_;
    }
  }
  ws_ = 0;
}


void Node::setDepth(UInt depth)
{
  depth_ = depth;
}


void Node::setId(UInt id)
{
  id_ = id;
}


void Node::setLb(double value)
{
  lb_ = value;
}


void Node::setWarmStart (WarmStartPtr ws) 
{ 
  if (ws) {
  ws_ = ws; 
  ws_->incrUseCnt();
  } else {
    removeWarmStart();
    ws_ = 0;
  }
}


void Node::undoPMods(ProblemPtr p)
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;

  // explicitely request the const_reverse_iterator for rend():
  // for bug in STL C++ standard
  // http://stackoverflow.com/questions/2135094/gcc-reverse-iterator-comparison-operators-missing
  ModificationRConstIterator rend = pMods_.rend();  


  // first undo the mods that were added while processing the node.
  for (mod_iter=pMods_.rbegin(); mod_iter!= rend; ++mod_iter) {
    mod = *mod_iter;
    mod->undoToProblem(p);
  } 

  // now undo the mods that were used to create this node from its parent.
  if (branch_) {
    for (mod_iter=branch_->pModsRBegin(); mod_iter!=branch_->pModsREnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod->undoToProblem(p);
    }
  }
}


void Node::undoRMods(RelaxationPtr rel)
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;

  // explicitely request the const_reverse_iterator for rend():
  // for bug in STL C++ standard
  // http://stackoverflow.com/questions/2135094/gcc-reverse-iterator-comparison-operators-missing
  ModificationRConstIterator rend = rMods_.rend();  


  // first undo the mods that were added while processing the node.
  for (mod_iter=rMods_.rbegin(); mod_iter!= rend; ++mod_iter) {
    mod = *mod_iter;
    mod->undoToProblem(rel);
  } 

  // now undo the mods that were used to create this node from its parent.
  if (branch_) {
    for (mod_iter=branch_->rModsRBegin(); mod_iter!=branch_->rModsREnd(); 
        ++mod_iter) {
      mod = *mod_iter;
      mod->undoToProblem(rel);
    }
  }
}


void Node::undoRModsTrans(RelaxationPtr rel)
{
  ModificationRConstIterator mod_iter;
  ModificationPtr mod;
  ProblemPtr p=0;

  // explicitely request the const_reverse_iterator for rend():
  // for bug in STL C++ standard
  // http://stackoverflow.com/questions/2135094/gcc-reverse-iterator-comparison-operators-missing
  ModificationRConstIterator rend = rMods_.rend();
  ModificationPtr pmod1, mod2;

  // first undo the mods that were added while processing the node.
  for (mod_iter=rMods_.rbegin(); mod_iter!= rend; ++mod_iter) {
    mod = *mod_iter;
    //converting modifications applicable for one relaxation to another
    pmod1 = mod->fromRel(rel, p);
    if (pmod1) {
      mod2 = pmod1->toRel(p, rel);
      mod2->undoToProblem(rel);
      delete mod2; mod2 = 0;
    } else {
      mod->undoToProblem(rel);
    }
    delete pmod1; pmod1 = 0;
  }

  // now undo the mods that were used to create this node from its parent.
  if (branch_) {
    for (mod_iter=branch_->rModsRBegin(); mod_iter!=branch_->rModsREnd();
        ++mod_iter) {
      mod = *mod_iter;
      pmod1 = mod->fromRel(rel, p);
      if (pmod1) {
        mod2 = pmod1->toRel(p, rel);
        mod2->undoToProblem(rel);
        delete mod2; mod2 = 0;
      } else {
        mod->undoToProblem(rel);
      }
      delete pmod1; pmod1 = 0;
    }
  }
}


void Node::undoMods(RelaxationPtr rel, ProblemPtr p)
{
  undoPMods(p);
  undoRMods(rel);
}


void Node::updateBrCands(UInt index) {
  brCands_.push_back(index);
}


void Node::updateLastStrBranched(UInt index, double value) {
  if (index >= lastStrBranched_.size()) {
    lastStrBranched_.push_back(value);
  } else {
    lastStrBranched_[index] = value;
  }
}


void Node::updatePCDown(UInt index, double value) {
  if (index >= pseudoDown_.size()) {
    pseudoDown_.push_back(value);
  } else {
    pseudoDown_[index] = value;
  }
}


void Node::updatePCUp(UInt index, double value) {
  if (index >= pseudoUp_.size()) {
    pseudoUp_.push_back(value);
  } else {
    pseudoUp_[index] = value;
  }
}


void Node::updateTimesDown(UInt index, double value) {
  if (index >= timesDown_.size()) {
    timesDown_.push_back(value);
  } else {
    timesDown_[index] = value;
  }
}


void Node::updateTimesUp(UInt index, double value) {
  if (index >= timesUp_.size()) {
    timesUp_.push_back(value);
  } else {
    timesUp_[index] = value;
  }
}


void Node::write(std::ostream &out) const
{
  out << "Node ID: " << id_ << " at depth: " << depth_;
  if (parent_) { 
    out << " has parent ID: " << parent_->getId()
        << " lb = " << lb_ << " tbScore_ = " << tbScore_ << std::endl;
  }
  out << std::endl;
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
