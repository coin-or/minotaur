// ===========================================================================
// $Date: 2010-03-07 22:35:34 -0600 (Sun, 07 Mar 2010) $
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: uuid.hpp 635 2010-03-08 04:37:15Z xzhen $
// ===========================================================================
#ifndef UUID_HH_
#define UUID_HH_

#include <map>
#include <list>
#include <iostream>
#include <iomanip>

namespace UUID{
  using std::list;
  
  // generate a unique object id in size_t
  // the uuid will start from seed, ie, seed is the minimum uuid
  class uuid{
  private:
    
    size_t seed_;
    size_t coid_;
    list<size_t> log_;
    
  public:
    typedef list<size_t>::iterator       iterator;  
    typedef list<size_t>::const_iterator const_iterator;
    
    // iterator support
    iterator begin() { return log_.begin(); }
    iterator end()   { return log_.end();   }
    const_iterator begin() const { return log_.begin(); }
    const_iterator end()   const { return log_.end(); }
    
    size_t size()const{return log_.size();};
    uuid(){seed_ = 1000; coid_ = 1000;};
    uuid(const size_t seed){seed_=seed; coid_=seed;};
    size_t next();
    void reset(){coid_ = seed_; log_.clear();};
  };
}
#endif
