//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file UnoEngine.cpp
 * \brief Define UnoEngine class for solving NLPs using Uno.
 * \author Sumanto Kar, IE & OR, Indian Institute of Technology Bombay
 */
#include "UnoEngine.h"
#include <iomanip>

#include "Constraint.h"
#include "Environment.h"
#include "HessianOfLag.h"
#include "MinotaurConfig.h"
#include "Objective.h"
#include "Option.h"
#include "Problem.h"
#include "ProblemSize.h"
#include "Timer.h"
#include "Uno.hpp"
#include "UnoModel.h"
#include "Variable.h"
#include "ingredients/constraint_relaxation_strategy/ConstraintRelaxationStrategyFactory.hpp"
#include "ingredients/globalization_mechanism/GlobalizationMechanismFactory.hpp"
#include "linear_algebra/RectangularMatrix.hpp"
#include "linear_algebra/SparseVector.hpp"
#include "linear_algebra/SymmetricMatrix.hpp"
#include "model/Model.hpp"
#include "model/ModelFactory.hpp"
#include "optimization/EvaluationErrors.hpp"
#include "symbolic/CollectionAdapter.hpp"
#include "symbolic/Concatenation.hpp"
#include "tools/Infinity.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Minotaur
{
const std::string UnoEngine::me_ = "UnoEngine: ";

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
UnoSolution::UnoSolution()
  : Solution(),
    dualXLow_(0),
    dualXUp_(0)
{
}

UnoSolution::UnoSolution(const double* x, double objval, ProblemPtr problem)
  : Solution(objval, x, problem),
    dualXLow_(0),
    dualXUp_(0)
{
}

UnoSolution::UnoSolution(ConstUnoSolPtr sol)
{
  n_ = sol->n_;
  m_ = sol->m_;

  if (sol->x_) {
    x_ = new double[n_];
    std::copy(sol->x_, sol->x_ + n_, x_);
  } else {
    x_ = 0;
  }
  if (sol->dualCons_) {
    dualCons_ = new double[m_];
    std::copy(sol->dualCons_, sol->dualCons_ + m_, dualCons_);
  } else {
    dualCons_ = 0;
  }

  if (sol->dualX_) {
    dualX_ = new double[n_];
    std::copy(sol->dualX_, sol->dualX_ + n_, dualX_);
  } else {
    dualX_ = 0;
  }

  if (sol->dualXLow_) {
    dualXLow_ = new double[n_];
    std::copy(sol->dualXLow_, sol->dualXLow_ + n_, dualXLow_);
  } else {
    dualXLow_ = 0;
  }

  if (sol->dualXUp_) {
    dualXUp_ = new double[n_];
    std::copy(sol->dualXUp_, sol->dualXUp_ + n_, dualXUp_);
  } else {
    dualXUp_ = 0;
  }

  consViol_ = INFINITY;
  objValue_ = INFINITY;
}

UnoSolution::~UnoSolution()
{
  if (dualXLow_) {
    delete[] dualXLow_;
  }
  if (dualXUp_) {
    delete[] dualXUp_;
  }
  // dualX_ is freed in base class dtor.
}

void UnoSolution::setDualOfVars(const double* lower, const double* upper)
{
}

void UnoSolution::write(std::ostream& out) const
{
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

UnoWarmStart::UnoWarmStart()
  : sol_(0)
{
}

/// Copy constructor. Creates a full copy, not just copies pointers.
UnoWarmStart::UnoWarmStart(ConstUnoWarmStartPtr ws)
{
}

UnoWarmStart::~UnoWarmStart()
{
}

UnoSolPtr UnoWarmStart::getPoint()
{
  return sol_;
}

bool UnoWarmStart::hasInfo()
{
  return false;
}

void UnoWarmStart::setPoint(UnoSolPtr sol)
{
}

void UnoWarmStart::write(std::ostream& out) const
{
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
UnoEngine::UnoEngine(EnvPtr env)
  : bndChanged_(false),
    consChanged_(false),
    env_(env),
    etol_(1e-7)
{
}

UnoEngine::~UnoEngine()
{
}

void UnoEngine::addConstraint(ConstraintPtr)
{
  consChanged_ = true;
}

void UnoEngine::changeBound(ConstraintPtr, BoundType, double)
{
  bndChanged_ = true;
}

void UnoEngine::changeBound(VariablePtr, BoundType, double)
{
  bndChanged_ = true;
}

void UnoEngine::changeBound(VariablePtr, double, double)
{
  bndChanged_ = true;
}

void UnoEngine::changeConstraint(ConstraintPtr, LinearFunctionPtr, double,
                                 double)
{
  consChanged_ = true;
}

void UnoEngine::changeConstraint(ConstraintPtr, NonlinearFunctionPtr)
{
  consChanged_ = true;
}

void UnoEngine::changeObj(FunctionPtr, double)
{
  consChanged_ = true;
}

void UnoEngine::clear()
{
}

void UnoEngine::disableStrBrSetup()
{
}

EnginePtr UnoEngine::emptyCopy()
{
}

void UnoEngine::enableStrBrSetup()
{
}

std::string UnoEngine::getName() const
{
  return "Uno";
}

ConstSolutionPtr UnoEngine::getSolution()
{
  if (sol_) {
    return sol_;
  }
  return SolutionPtr();  // NULL
}

ConstWarmStartPtr UnoEngine::getWarmStart()
{
}

WarmStartPtr UnoEngine::getWarmStartCopy()
{
}

double UnoEngine::getSolutionValue()
{
  return 0;
}

EngineStatus UnoEngine::getStatus()
{
  return status_;
}

void UnoEngine::load(ProblemPtr problem)
{
  /*if(problem_) {
          problem_->unsetEngine();
  }
  if(sol_) {
          delete sol_;
  }*/
  // std::cout << "\nThis is UnoEngine::Load Function\n";
  problem_ = problem;
  int n = problem_->getNumVars();
  int m = problem_->getNumCons();
  int kmax = 0;
  sol_ = new UnoSolution(0, INFINITY, problem);
  options = uno::Options::get_default_options(
      "/home/22m1526/minotaur/src/interfaces/uno.options");

  //set k_max value according to the number of variables
  if (n <= 1500) {
    kmax = n;
  } else if (n <= 5000) {
    kmax = n - (int)(m / 5);
  } else {
    kmax = 5000;
  }
  options["BQPD_kmax"] = std::to_string(kmax);
  const std::string& logger_level = options["logger"];

  uno::Logger::set_logger(logger_level);

  uno_model_ = std::make_unique<uno::UnoModel>(env_, problem_, sol_, options);
  // std::cout << "\nUno Model pointer created\n";
  // set the status of the engine to unknown.
  status_ = EngineUnknownStatus;
  bndChanged_ = true;
  consChanged_ = true;
  problem->calculateSize();
  setOptionsForProb_();
  problem->setEngine(this);
  // justLoaded_ = true;
}

void UnoEngine::loadFromWarmStart(const WarmStartPtr ws)
{
}

void UnoEngine::negateObj()
{
  // consChanged_ = true;
}

void UnoEngine::removeCons(std::vector<ConstraintPtr>&)
{
}

void UnoEngine::resetIterationLimit()
{
}

void UnoEngine::setIterationLimit(int limit)
{
}

void UnoEngine::setOptionsForProb_()
{
}

void UnoEngine::setOptionsForRepeatedSolve()
{
}

void UnoEngine::setOptionsForSingleSolve()
{
}

EngineStatus UnoEngine::solve()
{
  initial_iterate = std::make_shared<uno::Iterate>(
      uno_model_->number_variables, uno_model_->number_constraints);
  // std::cout << "\nThis is UnoEngine::solve()\n";
  // std::cout << "\nNo. of variables=" << uno_model_->number_variables <<
  //"\n"; std::cout << "\nNo. of constraints=" <<
  // uno_model_->number_constraints
  //<< "\n";
  //  uno::Model model_ = new UnoModel(env_, problem_, sol_);
  //

  // uno_model_->unoOptions_(options);
  uno_model_->initial_primal_point(initial_iterate->primals);
  uno_model_->project_onto_variable_bounds(initial_iterate->primals);
  uno_model_->initial_dual_point(initial_iterate->multipliers.constraints);

  initial_iterate->feasibility_multipliers.reset();
  model_ = uno::ModelFactory::reformulate(std::move(uno_model_),
                                          *initial_iterate, options);
  // model_ = uno_model_;
  auto constraint_relaxation_strategy =
      uno::ConstraintRelaxationStrategyFactory::create(*model_, options);
  auto globalization_mechanism = uno::GlobalizationMechanismFactory::create(
      *constraint_relaxation_strategy, options);
  uno_ = std::make_unique<uno::Uno>(*globalization_mechanism, options);
  if (&model_ == nullptr) {
    std::cerr << "Model is null!" << std::endl;
    // return;
  }
  if (&initial_iterate == nullptr) {
    std::cerr << "Iterate is null!" << std::endl;
    // return;
  }
  if (&options == nullptr) {
    std::cerr << "Options are null!" << std::endl;
    // return;
  }

  uno::Result result = uno_->solve(*model_, *initial_iterate, options);
  // status_ = ProvenLocalOptimal;
  status_ = ProvenLocalOptimal;
  const bool print_solution = true;
  result.print(print_solution);

  return status_;
}

void UnoEngine::fillStats(std::vector<double>& nlpStats)
{
}

void UnoEngine::writeStats(std::ostream& out) const
{
}
}  // namespace Minotaur

namespace uno
{
UnoModel::UnoModel(Minotaur::EnvPtr env, Minotaur::ProblemPtr problem,
                   Minotaur::UnoSolPtr sol, const Options& options)
  : Model(env->getOptions()->findString("problem_file")->getValue(),
          problem->getNumVars(), problem->getNumCons(), this->obj_sense_),
    eqConsColl_(this->eqCons_),
    ineqConsColl_(this->ineqCons_),
    lbVarColl_(this->lbVars_),
    ubVarColl_(this->ubVars_),
    singleLbVarColl_(this->singleLbVar_),
    singleUbVarColl_(this->singleUbVar_)
{
  varStat_.reserve(this->number_variables);

  p_ = problem;
  sol_ = sol;
  unoTol_ = options.get_double("tolerance");

  std::cout << std::fixed << std::setprecision(15);
  // std::cout << std::scientific << std::setprecision(15) << unoTol_
  //           << "UnoTol";
  if (p_->getObjective()->getObjectiveType() == Minotaur::Maximize) {
    this->obj_sense_ = -1.;
  } else {
    this->obj_sense_ = 1.;
  }
  //std ::cout << "Objective Sense=" << this->obj_sense_;
  // std::cout << "\nUnoModel::UnoModel:Model() Called\n";
  primals_ = new double[problem->getNumVars()];  // Allocate memory
  duals_ = new double[problem->getNumCons()];    // Allocate memory

  size_t index = 0;  // Index for the primals_ array
  for (Minotaur::VariableConstIterator vit = p_->varsBegin();
       vit != p_->varsEnd(); ++vit, ++index) {
    primals_[index] = (*vit)->getInitVal();  // Assign the initial value
  }
  // const double* initial_point;
  // sol_->setPrimal(sol->getPrimal());
  assert(sol_);
  // std::cout << "sol=" << sol_->getPrimal() << "\n";
  // initial_point = sol_->getPrimal();
  // std::cout << "primal=" << sol->getPrimal();
  // initial_point = sol_->getDualOfCons();

  // std::cout << "dual=" << initial_point;
  // std::fill(primals_, primals_ + problem->getNumVars(),
  //           0.0);  // Initialize the primals

  std::fill(duals_, duals_ + problem->getNumCons(),
            0.0);  // Initialize the duals
  // unoTol_ = 10e-15;  // setting the tolerance for UnoModel class
}
UnoModel::~UnoModel()
{
  // std::cout <<
  //"\n==========================================================="
  //"====\n";
  // std::cout << "\nUnoModel::~UnoModel() Called\n";
  // std::cout <<
  //"\n==========================================================="
  //"====\n";
}

double UnoModel::evaluate_objective(const Vector<double>& x) const
{
  // std::cout << "\n=========Evaluate Objective Starts=========\n";
  if (x.size() == 0) {
    std::cerr << "Error: Vector is empty." << std::endl;
    return 0;
  }

  const double* data = x.data();
  if (!data) {
    std::cerr << "Error: Data pointer is null." << std::endl;
    return 0;
  }

  // std::cout << "X address: " << static_cast<const void*>(x.data()) << "\n";
  // std::cout << "X Size: " << x.size() << "\n";

  // Verify that the pointer address and values remain stable across accesses
  // for (size_t i = 0; i < x.size(); ++i) {
  // std::cout << "X[" << i << "] = " << data[i] << " at address "
  //<< static_cast<const void*>(&data[i]) << "\n";
  //}
  int err = 0;
  double objval = p_->getObjValue(x.data(), &err);
  if (err != 0) {
    throw std::runtime_error("Error evaluating objective at this point");
  }
  // std::cout << "Objective Value:" << objval << "\n";
  // std::cout << "\n=========Evaluate Objective Ends=========\n";
  return obj_sense_ * objval;
}

void UnoModel::evaluate_constraints(const Vector<double>& x,
                                    std::vector<double>& constraints) const
{
  int err = 0;
  size_t i = 0;
  /*for (ConstraintConstIterator cit=p_->consBegin(); cit!=p_->consEnd();
  ++cit) { constraints[i] = (*cit)->evaluate(x, err);
  }*/
  // std::cout << "\n=========Evaluate Constraints starts=========\n";
  for (size_t i = 0; i < x.size(); ++i) {
    // std::cout << "X[" << i << "] = " << x[i] << " at address "
    //<< static_cast<const void*>(&x[i]) << "\n";
  }
  for (Minotaur::ConstraintConstIterator it = p_->consBegin();
       it != p_->consEnd(); ++it, ++i) {
    // std::cout<<x.data();
    constraints[i] = (*it)->getActivity(x.data(), &err);
    // std::cout << "constraints[" << i << "]=" << constraints[i] << "\n";
    // std::cout << "X pointer=" << x.data() << "\n";
    //  vio = std::max(act - (*it)->getUb(), (*it)->getLb() - act);
    //  vio = std::max(vio, 0.);
    //  if(vio > etol_ || e != 0) {
    //    break;
    //  }
  }
  // std::cout << "\n=========Evaluate Constraints Ends=========\n";
}

// sparse gradient
void UnoModel::evaluate_constraint_gradient(
    const Vector<double>& x, size_t constraint_index,
    SparseVector<double>& gradient) const
{
  // std::cout << "\n=========Evaluate Constraint Gradient starts=========\n";

  for (size_t i = 0; i < x.size(); ++i) {
    // std::cout << "X[" << i << "] = " << x[i] << " at address "
    //<< static_cast<const void*>(&x[i]) << "\n";
  }
  Minotaur::ConstraintPtr constraint = p_->getConstraint(constraint_index);
  size_t numVars = p_->getNumVars();
  int err = 0;

  // Temporary storage for gradient values (initialize to zero)
  std::vector<double> tempGradient(numVars, 0.0);

  // Check if there's a NonlinearFunction and evaluate it accordingly
  if (auto f = constraint->getFunction()) {
    f->evalGradient(x.data(), tempGradient.data(), &err);
  }

  // Print temporary gradient values
  // std::cout << "Computed constraint gradient values:\n";
  for (size_t i = 0; i < numVars; ++i) {
    // std::cout << "tempGradient[" << i << "] = " << tempGradient[i] << "\n";
  }

  // Clear and insert values into uno::SparseVector
  gradient.clear();
  for (size_t i = 0; i < numVars; ++i) {
    if (abs(tempGradient[i]) >
        unoTol_) {  // Only store non-zero elements in the sparse vector
      gradient.insert(i, tempGradient[i]);
    }
  }
  // std::cout << "\n=========Evaluate Constraint Gradient Ends=========\n";
}

void UnoModel::evaluate_constraint_jacobian(
    const Vector<double>& x,
    RectangularMatrix<double>& constraint_jacobian) const
{
  size_t numConstraints = p_->getNumCons();
  size_t numVars = p_->getNumVars();

  // Clear the constraint_jacobian
  for (size_t i = 0; i < numConstraints; ++i) {
    constraint_jacobian[i]
        .clear();  // Assuming clear() is available on SparseVector
  }

  // Evaluate each constraint's gradient and fill the Jacobian
  for (size_t constraint_index = 0; constraint_index < numConstraints;
       ++constraint_index) {
    this->evaluate_constraint_gradient(x, constraint_index,
                                       constraint_jacobian[constraint_index]);
  }
}

void UnoModel::evaluate_objective_gradient(
    const Vector<double>& x, SparseVector<double>& gradient) const
{
  // std::cout << "\n=========Evaluate Objective Gradient starts=========\n";
  for (size_t i = 0; i < x.size(); ++i) {
    // std::cout << "X[" << i << "] = " << x[i] << " at address "
    //<< static_cast<const void*>(&x[i]) << "\n";
  }
  size_t numVars = p_->getNumVars();
  int err = 0;

  // Temporary storage for objective gradient values, initialized to zero
  std::vector<double> tempGradient(numVars, 0.0);
  // std::cout << std::endl;

  // Minotaur-specific way to evaluate the objective gradient
  p_->getObjective()->getFunction()->evalGradient(x.data(),
                                                  tempGradient.data(), &err);

  if (err != 0) {
    throw std::runtime_error(
        "Error evaluating objective gradient at this point");
  }

  // Print the objective gradient values
  // std::cout << "Objective gradient values: ";
  for (size_t i = 0; i < numVars; ++i) {
    // std::cout << tempGradient[i] << " ";
  }
  // std::cout << std::endl;

  // Clear and insert values into uno::SparseVector
  gradient.clear();
  size_t count = 0;  // to count no of zeros
  for (size_t i = 0; i < numVars; ++i) {
    if (abs(tempGradient[i]) >
        unoTol_) {  // Only store non-zero elements in the sparse vector
      gradient.insert(i, obj_sense_ * tempGradient[i]);
    }
  }

  // std::cout << "\n=========Evaluate Objective Gradient ends=========\n";
}

/*void UnoModel::set_number_hessian_nonzeros() {

}*/

size_t UnoModel::number_objective_gradient_nonzeros() const
{
  return p_->getObjective()->getFunction()->getNumVars();
}

size_t UnoModel::number_jacobian_nonzeros() const
{
  return p_->getNumJacNnzs();
}

size_t UnoModel::number_hessian_nonzeros() const
{
  return p_->getNumHessNnzs();
}

const Collection<size_t>& UnoModel::get_equality_constraints() const
{
  return eqConsColl_;
}

const Collection<size_t>& UnoModel::get_inequality_constraints() const
{
  return ineqConsColl_;
}

const std::vector<size_t>& UnoModel::get_linear_constraints() const
{
  return linCons_;
}

const SparseVector<size_t>& UnoModel::get_slacks() const
{
  return this->slacks_;
}

const Collection<size_t>& UnoModel::get_single_lower_bounded_variables() const
{
  return singleLbVarColl_;
}

const Collection<size_t>& UnoModel::get_single_upper_bounded_variables() const
{
  return singleUbVarColl_;
}

const Collection<size_t>& UnoModel::get_lower_bounded_variables() const
{
  return lbVarColl_;
}

const Collection<size_t>& UnoModel::get_upper_bounded_variables() const
{
  return ubVarColl_;
}
/*
bool are_all_zeros(const Vector<double> &multipliers) {
        for (size_t i = 0; i < multipliers.size(); ++i) {
                if (abs(multipliers[i]) > unoTol_) {
                        return false;  // Return false if any element is not
zero
                }
        }
        return true;  // Return true if all elements are zero
}
*/
double* UnoModel::pullXToBnds_(const Vector<double>& x, size_t n) const
{
  double* xx = new double[n];
  for (int ii = 0; ii < n; ++ii) {
    if (x[ii] < p_->getVariable(ii)->getLb()) {
      xx[ii] = p_->getVariable(ii)->getLb();
    } else if (x[ii] > p_->getVariable(ii)->getUb()) {
      xx[ii] = p_->getVariable(ii)->getUb();
    } else {
      xx[ii] = x[ii];
    }
  }
  return xx;
}

void UnoModel::evaluate_lagrangian_hessian(
    const Vector<double>& x, double objective_multiplier,
    const Vector<double>& multipliers,
    SymmetricMatrix<size_t, double>& hessian) const
{
  // hessian.print(std::cout);
  // std::cout << "\n=========evaluate_lagrangian_hessian starts=========\n";
  //  Scale the objective multiplier based on the problem's objective sign
  int error = 0;
  double* ex = NULL;  // it will be assigned to xx if evalWithinBnds_ is true,
                      // otherwise to x.
  double* xx = NULL;
  size_t nnz_h_lag = p_->getNumHessNnzs();
  static std::vector<Minotaur::UInt> iRow(nnz_h_lag), jCol(nnz_h_lag);
  p_->getHessian()->fillRowColIndices(jCol.data(), iRow.data());
  for (size_t k = 0; k < nnz_h_lag; ++k) {
    // std::cout << "iRow=" << iRow[k];
  }
  assert(hessian.capacity >= nnz_h_lag);
  // if (x.empty() && multipliers.empty()) // && hessian = nullptr)
  //{
  //  p_->getHessian()->fillRowColIndices(iRow.data(), jCol.data());
  //} else
  if (!x.empty())  // && hessian != nullptr)
  {
    if (evalWithinBnds_) {
      xx = pullXToBnds_(x, x.size());
      ex = xx;
    } else {
      ex = const_cast<double*>(x.data());
    }

    // Create a vector to hold Hessian values
    static std::vector<double> values(nnz_h_lag);
    int error = 0;
    objective_multiplier *= this->obj_sense_;
    p_->getHessian()->fillRowColValues(ex, -1 * objective_multiplier,
                                       multipliers.data(), values.data(),
                                       &error);

    // Handle potential errors in Hessian evaluation
    // if (error != 0)
    //{
    //  throw std::runtime_error("Error occurred during Hessian
    //  evaluation.");
    //}

    // Populate the `hessian` matrix with computed values
    hessian.reset();
    // values[0] = -values[0];
    // for (size_t i = 0; i < this->number_variables; ++i) {
    for (size_t k = 0; k < nnz_h_lag; ++k) {
      // std::cout << values[k] << " values" << multipliers[k] << " "
      //           << std ::endl;
      // if (jCol[k] == i && abs(values[k]) >= unoTol_) {
      hessian.insert(-values[k], iRow[k], jCol[k]);
      //}
      // std::cout << values[k] << " Hi " << "iRow=" << iRow[k] <<
      // std::endl;
    }

    //  hessian.finalize_column(i);
    //}
    /*
        for (size_t k = 0; k < nnz_h_lag; ++k) {
          //values[0] = -values[0];
          /*hessian.insert(-values[0], 0, 1);
          hessian.insert(-values[1], 0, 4);
          hessian.insert(-values[2], 3, 4);
          hessian.insert(-values[3], 2, 5);
          hessian.insert(-values[4], 5, 5);
          hessian.insert(-values[5], 0, 6);
          hessian.insert(-values[6], 4, 6);
          hessian.insert(-values[7], 0, 8);
          hessian.insert(-values[8], 1, 9);
          hessian.insert(-values[9], 6, 10);
          hessian.insert(-values[10], 7, 11);
          */
    /*hessian.insert(-values[k], iRow[k], jCol[k]);
    //std::cout << values[k] << " Hi " << "iRow=" << iRow[k] << std::endl;
    hessian.finalize_column(k);
  }
  */
    // for (size_t k = 0; k < nnz_h_lag; ++k) {
    //   std::cout << " Hi " << values[k] << " Hi " << iRow[k] << " Hi "
    //             << jCol[k] << "\n";
    // }
    // std::cout << hessian.capacity << "capacity \n";
    // hessian.print(std::cout);
    //  Free memory if `pullXToBnds_` created a new array
    if (evalWithinBnds_ && ex != x.data()) {
      delete[] ex;
    }
    // //std::cout << "error = " << error << std::endl;
    // for (int i=0; i<problem_->getNumVars(); ++i) {
    //   //std::cout << std::setprecision(8) << "x["<<i<<"] = "<<x[i] <<
    //   std::endl;
    // }
    // for (int i=0; i<problem_->getHessian()->getNumNz(); ++i) {
    //   //std::cout << std::setprecision(8) << "h["<<i<<"] = "<<values[i]
    //   << std::endl;
    // }
    if (xx) {
      delete[] xx;
    }
  } else {
    // assert(!"one of x, lambda and values is NULL!");
  }
  // std::cout << "\n=========evaluate_lagrangian_hessian ends=========\n";
}

double UnoModel::variable_lower_bound(size_t variable_index) const
{
  return p_->getVariable(variable_index)->getLb();
}

double UnoModel::variable_upper_bound(size_t variable_index) const
{
  return p_->getVariable(variable_index)->getUb();
}

BoundType UnoModel::get_variable_bound_type(size_t variable_index) const
{
  return this->varStat_[variable_index];
}

double UnoModel::constraint_lower_bound(size_t constraint_index) const
{
  return p_->getConstraint(constraint_index)->getLb();
  // return this->consLb_[constraint_index];
}

double UnoModel::constraint_upper_bound(size_t constraint_index) const
{
  return p_->getConstraint(constraint_index)->getUb();
  // return 0.0;
}

FunctionType UnoModel::get_constraint_type(size_t constraint_index) const
{
  Minotaur::ConstraintPtr constraint = p_->getConstraint(constraint_index);
  Minotaur::FunctionType minofuncType = constraint->getFunctionType();

  switch (minofuncType) {
  case Minotaur::Linear:
    return uno::FunctionType::LINEAR;
  case Minotaur::Quadratic:
    return uno::FunctionType::NONLINEAR;
  case Minotaur::Nonlinear:
    return uno::FunctionType::NONLINEAR;
  default:
    // If Minotaur has more function types, handle them appropriately
    return uno::FunctionType::NONLINEAR;  // Or another suitable default
  }
}

BoundType UnoModel::get_constraint_bound_type(size_t constraint_index) const
{
  const auto* constraint = p_->getConstraint(constraint_index);

  // Get the lower and upper bounds of the constraint
  double lb = constraint->getLb();
  double ub = constraint->getUb();

  // Determine the bound type based on the values of lb and ub
  if (std::isinf(lb) && std::isinf(ub)) {
    return BoundType::UNBOUNDED;
  } else if (!std::isinf(lb) && std::isinf(ub)) {
    return BoundType::BOUNDED_LOWER;
  } else if (std::isinf(lb) && !std::isinf(ub)) {
    return BoundType::BOUNDED_UPPER;
  } else if (abs(lb - ub) < unoTol_) {
    return BoundType::EQUAL_BOUNDS;
  } else {
    return BoundType::BOUNDED_BOTH_SIDES;
  }
}

// initial primal point
void UnoModel::initial_primal_point(Vector<double>& x) const
{

  assert(x.size() >= this->number_variables);
  // std::cout << "\nInitializing primal point\n";
  std::copy(primals_, primals_ + this->number_variables, x.begin());

  // std::copy(initial_point, initial_point +  this->number_variables,
  // x.begin());
}

// initial dual point
void UnoModel::initial_dual_point(Vector<double>& multipliers) const
{
  assert(multipliers.size() >= this->number_constraints);
  // std::cout << "\nInitializing dual point" << std::endl;

  std::copy(duals_, duals_ + p_->getNumCons(), multipliers.begin());
}

void UnoModel::postprocess_solution(
    Iterate& /*iterate*/, TerminationStatus /*termination_status*/) const
{
  // do nothing
}

void UnoModel::genCons_()
{
  for (size_t constraint_index : Range(this->number_constraints)) {
    const auto* constraint = p_->getConstraint(constraint_index);
    const auto minofuncType = constraint->getFunctionType();
    this->consLb_[constraint_index] = constraint->getLb();
    this->consUb_[constraint_index] = constraint->getUb();
    switch (minofuncType) {
    case Minotaur::Linear:
      this->consType_[constraint_index] = LINEAR;
      this->linCons_.emplace_back(constraint_index);
    case Minotaur::Quadratic:
      this->consType_[constraint_index] = NONLINEAR;
    case Minotaur::Nonlinear:
      this->consType_[constraint_index] = NONLINEAR;
    default:
      // If Minotaur has more function types, handle them appropriately
      this->consType_[constraint_index] =
          NONLINEAR;  // Or another suitable default
    }
  }
  UnoModel::getBndType_(this->consLb_, this->consUb_, this->consStat_);
  // partition equality and inequality constraints
  for (size_t constraint_index : Range(this->number_constraints)) {
    if (this->get_constraint_bound_type(constraint_index) == EQUAL_BOUNDS) {
      this->eqCons_.emplace_back(constraint_index);
    } else {
      this->ineqCons_.emplace_back(constraint_index);
    }
  }
}

void UnoModel::genVars_()
{
  for (size_t variable_index : Range(this->number_variables)) {
    const auto* variable = p_->getVariable(variable_index);
    this->varLb_[variable_index] = variable->getLb();
    this->varUb_[variable_index] = variable->getUb();
    if (this->varLb_[variable_index] == this->varUb_[variable_index]) {
      WARNING << "Variable x" << variable_index << " has identical bounds\n";
      // this->fixed_variables.emplace_back(variable_index);
    }
  }

  UnoModel::getBndType_(this->varLb_, this->varUb_, this->varStat_);
  // figure out the bounded variables
  for (size_t variable_index : Range(this->number_variables)) {
    const BoundType status = this->get_variable_bound_type(variable_index);
    if (status == BOUNDED_LOWER || status == BOUNDED_BOTH_SIDES) {
      this->lbVars_.emplace_back(variable_index);
      if (status == BOUNDED_LOWER) {
        this->singleLbVar_.emplace_back(variable_index);
      }
    }
    if (status == BOUNDED_UPPER || status == BOUNDED_BOTH_SIDES) {
      this->ubVars_.emplace_back(variable_index);
      if (status == BOUNDED_UPPER) {
        this->singleUbVar_.emplace_back(variable_index);
      }
    }
  }
}

void UnoModel::getBndType_(const std::vector<double>& lowBnds_,
                           const std::vector<double>& upper_bounds,
                           std::vector<BoundType>& status)
{
  assert(lowBnds_.size() == status.size());
  assert(upper_bounds.size() == status.size());
  // build the "status" vector as a mapping (map/transform operation) of the
  // "bounds" vector
  for (size_t index : Range(lowBnds_.size())) {
    if (lowBnds_[index] == upper_bounds[index]) {
      status[index] = EQUAL_BOUNDS;
    } else if (is_finite(lowBnds_[index]) && is_finite(upper_bounds[index])) {
      status[index] = BOUNDED_BOTH_SIDES;
    } else if (is_finite(lowBnds_[index])) {
      status[index] = BOUNDED_LOWER;
    } else if (is_finite(upper_bounds[index])) {
      status[index] = BOUNDED_UPPER;
    } else {
      status[index] = UNBOUNDED;
    }
  }
}
}  // namespace uno
