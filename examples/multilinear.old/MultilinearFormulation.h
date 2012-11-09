/*
 *     MINOTAUR -- It's only 1/2 bull
 *
 *     (C)opyright 2009-- The MINOTAUR Team
 */

#ifndef MINOTAURMULTILINEARFORMULATION_H
#define MINOTAURMULTILINEARFORMULATION_H

#include <boost/shared_ptr.hpp>

#include <vector>

#include "Formulation.h"
#include "Instance.h"
#include "Types.h"

namespace Minotaur {

  class MultilinearFormulation : public Formulation {

  public:
    enum RelaxationStrategy { CONVEX_HULL,
                              ROW_BY_ROW,
                              TERM_BY_TERM,
                              MCCORMICK };

  public:
    MultilinearFormulation(InstancePtr instance, RelaxationStrategy s = CONVEX_HULL);

    virtual ~MultilinearFormulation() {};

    RelaxationStrategy getRelaxationStrategy() const { return relaxStrategy_; }
    

  private:

    RelaxationStrategy relaxStrategy_;

    // mapping from formulation to original instance
    //XXX Not used yet.  Probably should be in Formulation base class?
    std::map<VariablePtr,VariablePtr> variableMapping_;

    //!!! Things to do...
    //  (make some helper function, keep some data storage private, so there
    //   aren't huge blocks of copied code in different methods).


    /// Helper functions
      
    /// This creates the convex hull of all multilinear terms
    void makeConvexHull();

    ///  
    void makeMcCormick();

    /// This creates convex hull of each multilinear row
    void makeRowByRow();

    // This creates the convex hull of each multilinear term
    void makeTermByTerm();


    // For extreme point enumeration
    void allExtreme(std::vector<int> &S, std::vector<double> &lb, std::vector<double> &ub,
                    std::vector<std::vector<double> > &E);    
    void visit(std::vector<int> &S, std::vector<double> &lb, std::vector<double> &ub, 
               int ix, std::vector<double> &val, std::vector<std::vector<double> > &E) ;

    /// Computes the bounds on a product of two variables
    void boundsOnProduct(ConstVariablePtr x1, ConstVariablePtr x2,
                         double &lb, double &ub);

    //XXX This should probably go in constraint.  Returns the ids of the
    //  (original) variables that appear in multilinear or quadratic in this 
    // constraint
    std::vector<int> nonlinearVarsInConstraint(const ConstraintPtr c);

  };

  typedef boost::shared_ptr<MultilinearFormulation> MultilinearFormulationPtr;
}


#endif
// Local Variables:
// mode: c++
// eval: (c-set-style "gnu")
// eval: (setq indent-tabs-mode nil)
// End:
