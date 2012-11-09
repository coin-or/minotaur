#ifndef VARIABLEHANDLER_HH_
#define VARIABLEHANDLER_HH_

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "variable.hpp"
#include "agent.hpp"

using namespace std;
using boost::shared_ptr;
using boost::scoped_ptr;
using UUID::uuid;

class variableHandler: public genobj{
private:
  agent<variable>*  avp_;
  vector<variable*> vov_;
public:
  explicit variableHandler(agent<variable>* avp = NULL)
    :genobj(0,"vh"), avp_(avp){};
  
  explicit variableHandler(agent<variable>* avp, 
			   scoped_ptr<uuid>  &uid,
			   const size_t n, 
			   const char *familyName, 
			   const double lb, const double ub);
  
  size_t size()const{return vov_.size();};
  const vector<variable*> release()const{return vov_;};
  
  variable* operator[](const size_t i) const {return vov_[i];};
  variableHandler& add(const variableHandler& other)
  {
    if(avp_ == NULL) avp_ = other.avp_;
    vov_.insert(vov_.end(), other.vov_.begin(), other.vov_.end()); 
    return *this;
  }
};
#endif
