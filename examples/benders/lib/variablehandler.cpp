#include "variablehandler.hpp"


variableHandler::variableHandler(agent<variable>*  avp, 
				 scoped_ptr<uuid>  &uid,
				 const size_t n, 
				 const char *familyName, 
				 const double lb, const double ub)
  :genobj(0,"vh"), avp_(avp)
{
  vov_.resize(n);
  for(size_t i=0;i<n;i++){
    variable x(lb,ub); 
    x.name(familyName); 
    x.id(uid->next());
    variable* pv = avp_->add(x);
    vov_[i] = pv;
  }
}
