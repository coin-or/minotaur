// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2012 The MINOTAUR Team.
// 

/**
 * \file BrCand.h
 * \author Ashutosh Mahajan, Argonne National Laboratory.
 * \brief Declare the classes BranchCand and BrVarCand for storing candidates
 * for branching.
 */

#ifndef MINOTAURBRCAND_H
#define MINOTAURBRCAND_H

#include <string>
#include "Types.h"

namespace Minotaur {

  class Handler;
  class Variable;
  typedef boost::shared_ptr<Handler> HandlerPtr;
  typedef boost::shared_ptr<Variable> VariablePtr;

  /**
   * A BranchCand object is something that can be branched upon. This class is
   * abstract. Examples are BrVarCand, BrSOS1Cand, BrSOS2Cand, BrHyperCand, 
   * BrGenCand etc.
   */
  class BrCand {
    public:    
      /// Constructor
      BrCand() {};

      /// Destroy
      virtual ~BrCand() {};

      /// Return the index in the pseudo cost array. If it is not in the
      /// array, return -1.
      virtual Int getPCostIndex() const { return pCostIndex_; };

      /// Get the distance of the current point from the branching constraint:
      /// down direction. For an integer constrained variable, x, it would be
      /// \f$ x - \lfloor x \rfloor \f$.
      virtual Double getDDist() = 0;

      /// Get the distance of the current point from the branching constraint:
      /// up direction
      virtual Double getUDist() = 0;

      /// Set score for this candidate. Used to compare two candidates. 
      virtual void setScore(const Double score) { score_ = score; };

      /// Get score for this candidate. Used to compare two candidates.
      virtual Double getScore() { return score_; };

      /// Return the handler that created it
      virtual void setHandler(HandlerPtr h) { h_ = h; };

      /// Return the handler that created it
      virtual HandlerPtr getHandler() { return h_; };

      /**
       * Set the preferred direction that will be processed first in the
       * branch-and-bound tree.
       */
      virtual void setDir(BranchDirection d) {prefDir_ = d;};

      /// Get the preferred direction.
      virtual BranchDirection getDir() const {return prefDir_;};

      /// Display for debugging.
      virtual std::string getName() const {return "default br-cand";};

    protected:
      /// index of the this candidate in the pseudo-cost array.
      Int pCostIndex_;

      /// Score
      Double score_;

      /// Handler that created it
      HandlerPtr h_;

      /// Which is preferred for processing next? UpBranch or DownBranch?
      BranchDirection prefDir_;
  };   

  /// BrVarCand class is derived from BrCand
  class BrVarCand : public BrCand {
    public:
      /// Constructor.
      BrVarCand(VariablePtr var, Int i, Double f);

      ~BrVarCand();

      // getDDist().
      Double getDDist();

      // getUDist().
      Double getUDist();

      VariablePtr getVar();

      std::string getName() const;
    private:
      /// The variable on which we should branch.
      VariablePtr var_;

      /// fractional part.
      Double f_;
  };
  typedef boost::shared_ptr<BrVarCand> BrVarCandPtr;


  /// comparison function to sort candidates in the decreasing order of their
  /// score. How to break ties?
  Bool CompareScore(BrCandPtr c1, BrCandPtr c2); 
  
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
