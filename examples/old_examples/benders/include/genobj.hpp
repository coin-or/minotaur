#ifndef GENERALOBJ_HH_
#define GENERALOBJ_HH_

#include <string>
#include <sstream>

using namespace std;
class genobj{
private:
  size_t id_;
  string family_;
public:
  genobj(){id_ = 0; family_ = "unknown";};
  genobj(const size_t id, const char *family) {id_ = id; family_=family;};
  virtual ~genobj(){};
  virtual void id(const size_t i){id_ = i;}; // set the ID
  virtual size_t id()const{return id_;};     // get the ID
  
  virtual void name(const char *family){family_ = family;};
  virtual void name(const string& family){family_ = family;};
  virtual string name(const char s='_')const
  {ostringstream ost; ost << family_ << s << id_; return ost.str();}
};

bool trim(string& str);

#endif
