//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file QuadTransformer.h
 * \brief Declare class for transformation of quadraic problem.
 * \author Mustafa Vora, IIT Bombay
 */

#ifndef MINOTAURQUADTRANSFORMER_H
#define MINOTAURQUADTRANSFORMER_H

#include "Types.h"
#include "Transformer.h"

namespace Minotaur {
  class Environment;
  class LinearHandler;
  class UnivarQuadHandler;
  class Problem;
  class Solution;
  class YEqQfBil;
  class YEqBivar;
  class YEqUnivar;
  typedef LinearHandler* LinearHandlerPtr;
  typedef UnivarQuadHandler* UnivarQuadHandlerPtr;
  typedef Solution* SolutionPtr;
  typedef const Solution* ConstSolutionPtr;


  /**
   * \brief Class for reformulating a problem using simple rules so that
   * handlers can be applied to it.
   */
  class QuadTransformer : public Transformer {
  public:
    /// Default Constructor.
    QuadTransformer();

    /// Constructor.
    QuadTransformer(EnvPtr env, ProblemPtr p);

    /// Destroy.
    ~QuadTransformer();

    // base class method.
    std::string getName() const;

    // base class method.
    SolutionPtr getSolOrig(ConstSolutionPtr sol, int &err);

    // base class method.
    SolutionPtr getSolTrans(ConstSolutionPtr sol, int &err);

    // base class method.
    void reformulate(ProblemPtr &newp, HandlerVector &handlers, int &status);


  private:
    static const std::string me_;

    YEqQfBil *yQfBil_;
    YEqBivar *yBivar_;
    YEqUnivar *yUnivar_;

    /**
     * \brief Reformulate the nonlinear constraints of the problem.
     *
     * \param [in] oldp Original problem.
     */
    void refCons_(ConstProblemPtr oldp);

    /**
     * \brief Reformulate the nonlinear objective of the problem.
     *
     * \param [in] oldp Original problem.
     */
    void refObj_(ConstProblemPtr oldp);

    void refQuadCons_(QuadraticFunctionPtr qf, LinearFunctionPtr lf,
                      LinearFunctionPtr &lf1);
  };
  typedef QuadTransformer* QuadTranPtr;
  typedef const QuadTransformer* ConstQuadTranPtr;

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

