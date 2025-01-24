//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file UnoModel.h
 * \brief Declare the UNO UnoModel that provides Uno with the gradient,
 * hessian and function evaluations.
 * \author Sumanto Kar, IE & OR, Indian Institute of Technology Bombay
 */
#ifndef UNO_ENGINE_MODEL_H
#define UNO_ENGINE_MODEL_H


#include <vector>

#include "linear_algebra/SparseVector.hpp"
#include "model/Model.hpp"
#include "symbolic/CollectionAdapter.hpp"
extern "C"
{
#include "asl_pfgh.h"
#include "getstub.h"
}
namespace uno
{
Level Logger::level = INFO;
class UnoModel : public Model
{
public:
  explicit UnoModel(Minotaur::EnvPtr env, Minotaur::ProblemPtr problem,
                    Minotaur::UnoSolPtr sol, const Options& options);
  ~UnoModel() override;

  [[nodiscard]] double evaluate_objective(
      const Vector<double>& x) const override;
  void evaluate_objective_gradient(
      const Vector<double>& x, SparseVector<double>& gradient) const override;
  void evaluate_constraints(const Vector<double>& x,
                            std::vector<double>& constraints) const override;
  void evaluate_constraint_gradient(
      const Vector<double>& x, size_t constraint_index,
      SparseVector<double>& gradient) const override;
  void evaluate_constraint_jacobian(
      const Vector<double>& x,
      RectangularMatrix<double>& constraint_jacobian) const override;
  void evaluate_lagrangian_hessian(
      const Vector<double>& x, double objective_multiplier,
      const Vector<double>& multipliers,
      SymmetricMatrix<size_t, double>& hessian) const override;

  [[nodiscard]] double variable_lower_bound(
      size_t variable_index) const override;
  [[nodiscard]] double variable_upper_bound(
      size_t variable_index) const override;
  [[nodiscard]] BoundType get_variable_bound_type(
      size_t variable_index) const override;
  [[nodiscard]] const Collection<size_t>& get_lower_bounded_variables()
      const override;
  [[nodiscard]] const Collection<size_t>& get_upper_bounded_variables()
      const override;
  [[nodiscard]] const SparseVector<size_t>& get_slacks() const override;
  [[nodiscard]] const Collection<size_t>& get_single_lower_bounded_variables()
      const override;
  [[nodiscard]] const Collection<size_t>& get_single_upper_bounded_variables()
      const override;

  [[nodiscard]] double constraint_lower_bound(
      size_t constraint_index) const override;
  [[nodiscard]] double constraint_upper_bound(
      size_t constraint_index) const override;
  [[nodiscard]] FunctionType get_constraint_type(
      size_t constraint_index) const override;
  [[nodiscard]] BoundType get_constraint_bound_type(
      size_t constraint_index) const override;
  [[nodiscard]] const Collection<size_t>& get_equality_constraints()
      const override;
  [[nodiscard]] const Collection<size_t>& get_inequality_constraints()
      const override;
  [[nodiscard]] const std::vector<size_t>& get_linear_constraints()
      const override;

  void initial_primal_point(Vector<double>& x) const override;
  void initial_dual_point(Vector<double>& multipliers) const override;
  void postprocess_solution(
      Iterate& iterate, TerminationStatus termination_status) const override;

  [[nodiscard]] size_t number_objective_gradient_nonzeros() const override;
  [[nodiscard]] size_t number_jacobian_nonzeros() const override;
  [[nodiscard]] size_t number_hessian_nonzeros() const override;

private:
  Minotaur::ProblemPtr p_;
  Minotaur::UnoSolPtr sol_;
  double* primals_;
  double* duals_;
  double unoTol_;
  double obj_sense_;
  SparseVector<size_t> slacks_{};
  std::vector<double> varLb_, varUb_, consLb_, consUb_;
  std::vector<size_t> eqCons_{};
  CollectionAdapter<std::vector<size_t>&> eqConsColl_;
  std::vector<size_t> ineqCons_{};
  CollectionAdapter<std::vector<size_t>&> ineqConsColl_;
  std::vector<size_t> lbVars_;
  CollectionAdapter<std::vector<size_t>&> lbVarColl_;
  std::vector<size_t> ubVars_;
  CollectionAdapter<std::vector<size_t>&> ubVarColl_;
  std::vector<size_t> singleLbVar_{};  // indices of the single lower-bounded
                                       // variables
  CollectionAdapter<std::vector<size_t>&> singleLbVarColl_;
  std::vector<size_t> singleUbVar_{};  // indices of the single upper-bounded
                                       // variables
  CollectionAdapter<std::vector<size_t>&> singleUbVarColl_;
  void genCons_();
  void genVars_();
  std::vector<size_t> linCons_;
  std::vector<BoundType> varStat_;
  std::vector<FunctionType> consType_;
  std::vector<BoundType> consStat_;
  static void getBndType_(const std::vector<double>& lowBnds_,
                          const std::vector<double>& uppBnds_,
                          std::vector<BoundType>& status);

  // UnoModel(const std::string& file_name);
  // [[nodiscard]] size_t compute_hessian_number_nonzeros(double
  // objective_multiplier, const Vector<double>& multipliers) const;
  [[nodiscard]] double getHessEntry_(const Vector<double>& x,
                                     const Vector<double>& multipliers,
                                     size_t row, size_t col,
                                     double objective_multiplier) const;
  double* pullXToBnds_(const Vector<double>& x, size_t n) const;
  bool evalWithinBnds_ = false;
};

}  // namespace uno
#endif  // UNO_UnoFunInterface_H
