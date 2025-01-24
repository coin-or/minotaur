// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2008 - 2025 The Minotaur Team.
// 

// /**
// \file WarmStart.h
// \brief Define the base class WarmStart for storing the warm starting
// information for different types of engines.
// \author Ashutosh Mahajan, Argonne National Laboratory
// */


#ifndef MINOTAURWARMSTART_H
#define MINOTAURWARMSTART_H

#include "Types.h"

namespace Minotaur {

  class WarmStart;
  typedef WarmStart* WarmStartPtr;
  typedef const WarmStart* ConstWarmStartPtr;

  // /** 
  // Warm starting information enables an engine to quickly resolve a problem
  // after that has been modified. 
  //
  // We save warm start information on all those
  // nodes of the branch-and-bound tree whose at least one child is saved for
  // later processing. If all the children of a node are pruned or if the node
  // has only one child and we decide to process it next, then we don't need
  // to save warm start information for that node.
  //
  // For now we save complete warm-start information on each active node. A more
  // memory efficient method is to save warm-start information for each node
  // by just storing the differences from the parent.
  // However, the benefits of saving the complete information are:
  //  -# Ease of coding.
  //  -# We can delete warm-start information of a node when it is processed.
  //  Thus, we have at most `A' nodes that have warm-start information saved
  //  on them, where `A' is the total number of active nodes in the tree.
  // */
  class WarmStart {
    public:
      /// Default constructor
      WarmStart() {cnt_ = 0;} ;

      /// Destroy
      virtual ~WarmStart() {} ;
      
      virtual void decrUseCnt()
      {--cnt_;} ;

      virtual int getUseCnt()
      {return cnt_;} ;
      
      /// Return true if warm start information is initialized, false
      /// otherwise.
      virtual bool hasInfo() = 0;

      virtual void incrUseCnt()
      {++cnt_;} ;
      
      /// Write to an output stream
      virtual void write(std::ostream &out) const = 0;

    protected:
      /**
       * Warm start information can be stored at different nodes of the
       * tree -- making it difficult to delete when it is no longer in use.
       * This variable keeps track of number of places (nodes for example) it
       * is in use. When it is zero, it is safe to delete it.
       */
      int cnt_;
  };
}
#endif

