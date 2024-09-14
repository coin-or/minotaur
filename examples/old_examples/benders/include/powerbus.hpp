#ifndef BUS_HH_
#define BUS_HH_

#include <iostream>
#include <map>
#include <assert.h>
using namespace std;

class bus{
private:
  size_t id_;
  double lon_, lat_;
  bool   windStatus_;
  double windShare_;
  bool   loadStatus_;
  map<size_t, double> load_;
  void   defaultInit(){windShare_ = false; loadStatus_ = false;windShare_ = 0.0;};
public:
  bus(){defaultInit();};
  bus(const size_t id, const double lon, const double lat)
  {defaultInit();id_=id;lon_=lon;lat_=lat;};
  
  void   id(const size_t id){id_ = id;};
  size_t id()const{return id_;};
  
  double load(const size_t time)
  {
    map<size_t,double>::const_iterator it = load_.find(time);
    if(it == load_.end()) return 0.0;
    else return load_[time];
  }
  
  void   load(const vector<size_t>& time, const vector<double>& val)
  {
    size_t n = time.size();
    assert(n == val.size());
    for(size_t i=0;i<n;i++) load_[time[i]] = val[i];
  }
  
  void   load(const size_t time, const double val) {load_[time] = val;};
  
  void   loadOn(){loadStatus_ = true;}
  void   loadOff(){loadStatus_ = false;}
  bool   loadActive(){return loadStatus_;};
  
  void   windOn(const double share){windStatus_ = true; windShare_ = share;};
  void   windOff(){windStatus_ = false;};
  bool   windActive()const{return windStatus_;};
  double windShare()const{return windShare_;};
  
  friend istream& operator>>(istream& fin, bus& b)
  {
    fin >> b.id_ >> b.lon_ >> b.lat_;
    return fin;
  }
  friend ostream& operator<<(ostream& os, bus& b)
  {
    os << b.id_  << ":"
       << b.lon_ << " "
       << b.lat_;
    if(b.windActive()) os << " W: " << b.windShare_;
    
    return os;
  }
};
#endif
