//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file SimpleTransformer.h
 * \brief Declare class for simple transformation of problem.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURSIMPLETRANSFORMER_H
#define MINOTAURSIMPLETRANSFORMER_H

#include "Transformer.h"
#include "Types.h"

namespace Minotaur
{
class CxUnivarHandler;
class CGraph;
class CNode;
class Environment;
class Engine;
class LinearHandler;
class LPEngine;
class Problem;
class QuadHandler;
class Solution;
class YEqCGs;
class YEqLFs;
class YEqVars;
class YEqQfBil;
typedef CxUnivarHandler* CxUnivarHandlerPtr;
typedef CGraph* CGraphPtr;
typedef LinearHandler* LinearHandlerPtr;
typedef QuadHandler* QuadHandlerPtr;
typedef Solution* SolutionPtr;
typedef const Solution* ConstSolutionPtr;
typedef LPEngine* LPEnginePtr;
typedef Engine* EnginePtr;

/**
 * \brief Class for reformulating a problem using simple rules so that
 * handlers can be applied to it.
 *
 * No multilinear terms are created. QuadHandler is used only for terms
 * \f$y=x_1x_2\f$. Squares etc. are handled by PowerHandler. ExpHandler
 * takes care of exponential functions, and LogHandler handles logarithms.
 * TrigHandler is used for trigonometric functions. Mainly used to teach
 * Ashu some global optimization.
 */
class SimpleTransformer : public Transformer
{
public:
  /// Default Constructor.
  SimpleTransformer();

  /// Constructor.
  SimpleTransformer(EnvPtr env, ProblemPtr p, LPEnginePtr bte, LPEnginePtr cute,
                    EnginePtr nlpe);

  /// Destroy.
  ~SimpleTransformer();

  // base class method.
  std::string getName() const;

  // base class method.
  SolutionPtr getSolOrig(ConstSolutionPtr sol, int& err);

  // base class method.
  SolutionPtr getSolTrans(ConstSolutionPtr sol, int& err);

  // base class method.
  void reformulate(ProblemPtr& newp, HandlerVector& handlers, int& status);

  void writeStats(std::ostream& out) const;

private:
  // Store statistics of reformulation
  struct RefStats {
    double time;  ///> Total time reformulation and convexity detection
    UInt nvars;   ///> Number of variables added
    UInt ncons;   ///> Number of constraints added
    UInt nconv;   ///> Number of convex constraints
    UInt objConv; ///> 0 : Linear objective
                  ///> 1 : Convex Quadratic Objective
                  ///> 2 : Nonconvex Quadratic Objective
  };

  static const std::string me_;

  LPEnginePtr bte_;

  LPEnginePtr cute_;

  EnginePtr nlpe_;

  RefStats stats_;

  YEqCGs* yBiVars_;
  YEqQfBil* yQfBil_;

  void absRef_(LinearFunctionPtr lfl, VariablePtr vl, double dl, VariablePtr& v,
               double& d);

  void bilRef_(LinearFunctionPtr lfl, VariablePtr vl, double dl,
               LinearFunctionPtr lfr, VariablePtr vr, double dr,
               LinearFunctionPtr& lf, VariablePtr& v, double& d);

  VariablePtr newBilVar_(VariablePtr vl, VariablePtr vr);

  /**
   * \brief Checks the convexity of quadratic problems
   */
  bool checkQuadConvexity_();

  void powKRef_(LinearFunctionPtr lfl, VariablePtr vl, double dl, double k,
                LinearFunctionPtr& lf, VariablePtr& v, double& d);

  /**
   * \brief Reformulate the nonlinear constraints of the problem.
   *
   * \param [in] oldp Original problem.
   */
  void refNonlinCons_(ConstProblemPtr oldp);

  /**
   * \brief Reformulate the nonlinear objective of the problem.
   *
   * \param [in] oldp Original problem.
   */
  void refNonlinObj_(ConstProblemPtr oldp);

  void refQuadCons_(QuadraticFunctionPtr qf, LinearFunctionPtr& lf,
                    char cons_type);

  /**
   * TODO
   */
  void recursRef_(const CNode* node, LinearFunctionPtr& lf, VariablePtr& v,
                  double& d);

  // Reset the statistics for transformer
  void resetStats_();

  void trigRef_(OpCode op, LinearFunctionPtr lfl, VariablePtr vl, double dl,
                VariablePtr& v, double& d);

  void uniVarRef_(const CNode* n0, LinearFunctionPtr lfl, VariablePtr vl,
                  double dl, LinearFunctionPtr& lf, VariablePtr& v, double& d);
};
typedef SimpleTransformer* SimpTranPtr;
typedef const SimpleTransformer* ConstSimpTranPtr;

} // namespace Minotaur

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
