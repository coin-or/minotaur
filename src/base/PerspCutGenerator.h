//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCutGenerator.h 
 * \Declare PerspCutGenerator class . 
 * \author Meenarli Sharma, Indian Institute of Technology Bombay 
 */

#ifndef MINOTAURPERSPCUTGENERATOR_H
#define MINOTAURPERSPCUTGENERATOR_H

#include <map>
#include <fstream>
using std::ofstream;
#include <string>
using std::string;

#include "Types.h"
#include "Problem.h"
#include "Solution.h"
#include "Cut.h"
#include "Relaxation.h"
#include "Environment.h"
#include "PerspCon.h"
#include "Logger.h"

namespace Minotaur {

  //typedef enum {
    //Cons = 0
  //} PrintType;

  //typedef enum {
    //Totalcuts = 0,
    //Cuts,
    //Violated,
    //Noviol
  //} PerspType;

  //typedef enum {
    //Duplicate = 0,
    //Notviolated
  //} CutFail;

  //struct PerspGenStats
  //{
    //UInt totalcuts;
    //UInt violated;
    //UInt noviol;
    //UInt perspcons;
    //UInt perspnotcons;
    //UInt perspconsidered;
    //double time;
  //};

  // Typedefs
  class PerspCutGenerator;
  typedef PerspCutGenerator* PerspCutGeneratorPtr;
  typedef const PerspCutGenerator* ConstPerspCutGeneratorPtr;
  //typedef PerspGenStats * PerspGenStatsPtr;
  //typedef PerspGenStats const * ConstPerspGenStatsPtr;

  /**
   * PerpsCutGenerator class generates perspective cuts from constraints.
   * For each constraint, we considere to get the perspective of the constraint.
   * We generate a linearization of perspective function at the current
   * solution.
   * Then, we add this linearization as an inequality to problem formulation.
   */
  class PerspCutGenerator {
  public:
    /// Default constructor.
    PerspCutGenerator();


    /// Constructor that uses a relaxation and a solution given.
    //MS: do we need both rel and p here?
    PerspCutGenerator(UInt relvars, ConstSolutionPtr sol, ConstConstraintPtr c,
                      ConstVariablePtr v);
    /// Destructor.
    ~PerspCutGenerator();

    /// Initialize data elements.
    //void initialize();

    /// Generate all perspective cuts.
    //bool generateAllCuts();

    /// Generate perspective cut.
    void generateCut();
    //ConstraintPtr generateCut();
    
    void gPCut(FunctionPtr f, double * y);
    //ConstraintPtr gPCut(FunctionPtr f, double * y);

    // Return function pointer of PC
    LinearFunctionPtr getPFunction() {return lf_;}

    // Return constant term of PC
    //double getConst();
    /// Add cut to the corresponding lists.
    //bool addCut(CutPtr cut);

    /// Check if the same cut is already included in the list.
    //bool checkExists(CutPtr cut);

    /// Calculates the violation for a given cut.
    //double violation(CutPtr cut);

    ///True if a new perspective cut violating current solution is found
    //bool newVioPC() const
    //{return newVioCut_;}

    /// Return const cut list.
    //CutVector getCutList() const
    //{return cutList_;}

    /// Return cuts that violates relaxation solution.
    //CutVector getViolatedCutList() const
    //{return viollist_;}

    /// Return violation list.
    //DoubleVector getViolList() const
    //{return viols_;}

    /// Return statistics of cut generator.
    //ConstPerspGenStatsPtr getStats() const
    //{return ConstPerspGenStatsPtr(stats_);}

    /// Check  if given solution satisfies integrality for given relaxation.
    //bool checkIntegral(RelaxationPtr p, ConstSolutionPtr s);

  private:
    // Environment.
    //EnvPtr env_;

    // Problem that cover cuts will be generated for.
    //ProblemPtr p_;

    // Relaxation that cover cuts will be generated for.
    RelaxationPtr rel_;

    // For log
    LoggerPtr logger_;

    // For log
    static const std::string me_;

    // List of cuts generated.
    //CutVector cutList_;

    // List of violated cuts.
    //CutVector viollist_;

    // List of violations for cuts corresponding in cut list.
    //DoubleVector viols_;

    //This is true is a new cut violating the current solution is added
     //bool newVioCut_;

    // List of constraints that we can obtain perspective cuts.
    // We call them perspective constraints.
    // PerspConPtr persplist_;

    /** Given (possibly fractional) solution.
     *  Cut will be designed to violate this solution.
     */
    ConstSolutionPtr s_;

    // Perspective constraints.
    //PerspConPtr conslist_;
    
    ConstConstraintPtr cp_;
    ConstVariablePtr bp_;

    // Violation tolerance.
    //double objtol_;


    // Statistics for perspective cut generator.
    //PerspGenStatsPtr stats_;

    // Hash map that is used to check if a cut is already created or not.
    //std::map< DoubleVector, UInt > cutmap_;

    // Integer tolerance.
    //double intTol_;

    /// Tolerance for accepting a new solution value: absolute threshold.
    //double solAbsTol_;
    //double solRelTol_;
    
    // General tolerance.
    //double eTol_;

    // Output file
    ofstream output_;

    // Output file name
    string outfile_;

    // Function of pc
    LinearFunctionPtr lf_;

    // Constant term of pc
    //double cnst_;

    // Number of variable in relaxation
    UInt rnv_;
  };
}

#endif // MINOTAURPERSPCUTGENERATOR_H




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
