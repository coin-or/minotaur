// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/**
 * \file ConBoundMod.cpp
 * \brief Implement the Modification class ConBoundMod, that is used to store
 * modifications to a bound on a constraint.
 * \author Meenarli Sharma, Indian Institute of Technology Bombay
 */

#include <iostream>

#include "MinotaurConfig.h"
#include "Engine.h"
#include "Problem.h"
#include "Relaxation.h"
#include "ConBoundMod.h"
#include "Constraint.h"
#include "NonlinearFunction.h"


using namespace Minotaur;


ConBoundMod::ConBoundMod(ConstraintPtr con, BoundType lu, double new_val)
  : lu_(lu),
    newVal_(new_val),
    con_(con)
{
  switch (lu) {
   case (Lower):
     oldVal_ = con->getLb();
     break;
   case (Upper):
     oldVal_ = con->getUb();
     break;
   default:
     break;
  }
}


ConBoundMod::~ConBoundMod()
{
  //con_.reset(); need to check
}

/// TO DO
//ModificationPtr ConBoundMod::fromRel(RelaxationPtr rel, ProblemPtr) const
//{
  //ConBoundModPtr mod = (ConBoundModPtr) new ConBoundMod(
                                        //rel->getOriginalVar(con_),
                                        //lu_, newVal_);
  //mod->oldVal_ = oldVal_;
  //return mod;
//}


ConstraintPtr ConBoundMod::getCon() const
{
  return con_;
}


BoundType ConBoundMod::getLU() const
{
  return lu_;
}


double ConBoundMod::getNewVal() const
{
  return newVal_;
}


void ConBoundMod::applyToProblem(ProblemPtr problem) 
{
  problem->changeBound(con_, lu_, newVal_);
}

/// TO DO
//ModificationPtr ConBoundMod::toRel(ProblemPtr, RelaxationPtr rel) const
//{
  //ConBoundModPtr mod = (ConBoundModPtr) new ConBoundMod(
                                        //rel->getRelaxationVar(con_),
                                        //lu_, newVal_);
  //mod->oldVal_ = oldVal_;
  //return mod;
//}


void ConBoundMod::undoToProblem(ProblemPtr problem) 
{
  problem->changeBound(con_, lu_, oldVal_);
}


void ConBoundMod::write(std::ostream &out) const
{
  out << "con bound mod: "
      << "con name = " << con_->getName()
      << " bound type = ";
  if (lu_==Lower) {
    out << "lb"; 
  } else {
    out << "ub";
  }
  out << " old value = " << oldVal_
      << " new value = " << newVal_
      << std::endl;
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

ConBoundMod2::ConBoundMod2(ConstraintPtr con, double new_lb, double new_ub) 
  : newLb_(new_lb),
    newUb_(new_ub),
    con_(con)
{
  oldLb_ = con->getLb();
  oldUb_ = con->getUb();
}


ConBoundMod2::~ConBoundMod2()
{
  //con_.reset(); need to check
}

/// TO DO
//ModificationPtr ConBoundMod2::fromRel(RelaxationPtr rel, ProblemPtr) const
//{
  //ConBoundMod2Ptr mod = (ConBoundMod2Ptr) new ConBoundMod2(
                                          //rel->getOriginalVar(con_),
                                          //newLb_, newUb_);
  //mod->oldLb_ = oldLb_;
  //mod->oldUb_ = oldUb_;

  //return mod;
//}


ConstraintPtr ConBoundMod2::getCon() const
{
  return con_;
}


double ConBoundMod2::getNewLb() const
{
  return newLb_;
}


double ConBoundMod2::getNewUb() const
{
  return newUb_;
}


void ConBoundMod2::applyToProblem(ProblemPtr problem)
{
  problem->changeBound(con_, newLb_, newUb_);
}

//// TO DO
//ModificationPtr ConBoundMod2::toRel(ProblemPtr, RelaxationPtr rel) const
//{
  //ConBoundMod2Ptr mod = (ConBoundMod2Ptr) new ConBoundMod2(
                                          //rel->getRelaxationVar(con_),
                                          //newLb_, newUb_);
  //mod->oldLb_ = oldLb_;
  //mod->oldUb_ = oldUb_;

  //return mod;
//}


void ConBoundMod2::undoToProblem(ProblemPtr problem)
{
  problem->changeBound(con_, oldLb_, oldUb_);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

ConMod::ConMod(ConstraintPtr con, NonlinearFunctionPtr new_nlf) 
  : newNlf_(new_nlf),
    con_(con)
{
  oldNlf_ = con->getNonlinearFunction();
}


ConMod::~ConMod()
{
  //con_.reset(); need to check
}

ConstraintPtr ConMod::getCon() const
{
  return con_;
}


NonlinearFunctionPtr ConMod::getNewNlf() const
{
  return newNlf_;
}


void ConMod::applyToProblem(ProblemPtr problem) 
{
  problem->changeConstraint(con_, newNlf_);
}

/// TO DO
//ModificationPtr ConBoundMod::toRel(ProblemPtr, RelaxationPtr rel) const
//{
  //ConBoundModPtr mod = (ConBoundModPtr) new ConBoundMod(
                                        //rel->getRelaxationVar(con_),
                                        //lu_, newVal_);
  //mod->oldVal_ = oldVal_;
  //return mod;
//}


void ConMod::undoToProblem(ProblemPtr problem) 
{
  problem->changeConstraint(con_, oldNlf_);
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
