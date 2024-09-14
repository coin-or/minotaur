// ===========================================================================
// $Date: 2010-03-07 22:37:15 -0600 (Sun, 07 Mar 2010) $
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: uuid.cpp 634 2010-03-08 04:35:34Z xzhen $
// ===========================================================================
#include "uuid.hpp"

namespace UUID{
  size_t uuid::next()
  {
    size_t id = coid_;
    log_.push_back(id);
    ++coid_;
    return id;
  };
}
