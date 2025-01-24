//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file IpoptEngineTnlp.h
 * \brief Declare the IPOPT TNLP class that provides Ipopt with the gradient,
 * hessian and function evaluations.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */


#ifndef MINOTAURIPOPTENGINETNLP_H
#define MINOTAURIPOPTENGINETNLP_H

namespace Ipopt {
  class IpoptFunInterface : public TNLP
  {
  public:

    /// Default constructor.
    IpoptFunInterface(Minotaur::EnvPtr env,
                      Minotaur::ProblemPtr problem, 
                      Minotaur::IpoptSolPtr sol);

    /// default destructor.
    ~IpoptFunInterface();

    /// Copy the solution explicitly (without sharing pointers).
    void copySolution(Minotaur::IpoptSolPtr sol);

    /// Method to return the objective value.
    bool eval_f(Index n, const Number* x, bool new_x, 
                Number& obj_value);

    /// Method to return the constraint residuals.
    bool eval_g(Index n, const Number* x, bool new_x, 
                Index m, Number* g);

    /// Method to return the gradient of the objective.
    bool eval_grad_f(Index n, const Number* x, 
                     bool new_x, Number* grad_f);

    /** 
     * Method to return:
     *   1) The structure of the jacobian (if "values" is NULL)
     *   2) The values of the jacobian (if "values" is not NULL).
     */
    bool eval_jac_g(Index n, const Number* x, 
                    bool new_x, Index m, Index nele_jac,
                    Index* iRow, Index *jCol,
                    Number* values);

    /** 
     * Method to return:
     *   1) The structure of the hessian of the lagrangian (if "values" is
     *   NULL) 
     *   2) The values of the hessian of the lagrangian (if "values"
     *   is not NULL)
     */
    bool eval_h(Index n, const Number* x, bool new_x,
                Number obj_factor, Index m, 
                const Number* lambda, bool new_lambda, 
                Index nele_hess, Index* iRow, 
                Index* jCol, Number* values);


    /** 
     * This method is called when the algorithm is complete so the TNLP can
     * store/write the solution 
     */
    void finalize_solution(SolverReturn status, 
                           Index n, const Number* x, const Number* z_L, 
                           const Number* z_U, Index m, const Number* g, 
                           const Number* lambda, Number obj_value, 
                           const IpoptData* ip_data, 
                           IpoptCalculatedQuantities* ip_cq);

    /// Method to return the bounds for my problem.
    bool get_bounds_info(Index n, Number* x_l,
                         Number* x_u, Index m, 
                         Number* g_l, Number* g_u);

    /// Method to return some info about the nlp.
    bool get_nlp_info(Index& n, Index& m, 
                      Index& nnz_jac_g, Index&
                      nnz_h_lag, IndexStyleEnum& index_style);

    /// Get solution.
    Minotaur::IpoptSolPtr getSolution() {return sol_;}

    /// Get solution value.
    double getSolutionValue() const;

    /// Method to return the starting point for the algorithm.
    bool get_starting_point(Index n, bool init_x, 
                            Number* x, bool init_z, 
                            Number* z_L, Number* z_U,
                            Index m, bool init_lambda,
                            Number* lambda);

    /// Set solution.
    void setSolution(Minotaur::IpoptSolPtr sol) {sol_ = sol;}


  private:
    /// Copying is not allowed.
    IpoptFunInterface(const IpoptFunInterface &);

    /// Assignment is not allowed.
    IpoptFunInterface& operator=(const IpoptFunInterface&);

    /// If the variable is fixed, relax the upper bound by this much.
    double bOff_;

    /// If fabs(lb-ub)<bTol_ for a given variable, it is fixed to lb.
    double bTol_;

    /// Pull values of a variable to its bounds if it is not within bounds for
    /// function and derivative evaluations
    bool evalWithinBnds_;

    /// Where to put logs.
    Minotaur::LoggerPtr logger_;

    /// Problem that is being solved.
    Minotaur::ProblemPtr problem_;

    /**
     * The final solution obtained from Ipopt. We copy the solution from
     * Ipopt and not merely keep a pointer. The allocation and freeing of this
     * pointer is managed by IpoptEngine class. The size and allocation is not
     * changed within IpoptFunInterface
     */
    Minotaur::IpoptSolPtr sol_;

    /**
     * If x violates the lower or upperbounds on the variables, then function
     * evaluation or derivatives may give error (e.g. (x1)^1.852). It may be
     * desirable to first change x so that it is within the bounds. This
     * function does this. 
     * \param[in] x array of coordinates of the currect point.
     * \param[in] n size of x.
     * \param[out] A new array of size n with values same as x if it is in
     * bounds and bounds if any is violated.
     */
    double *pullXToBnds_(const Number* x, Index n);
  };
}
#endif
