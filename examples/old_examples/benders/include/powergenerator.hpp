#ifndef GENERATOR_HH_
#define GENERATOR_HH_

#include <iostream>
using namespace std;

class generator{
private:
  size_t id_;
  size_t bus_;
  double cap_;
  double unitCost_;
public:
  generator(){};
  generator(const size_t id, const size_t b) {id_=id;bus_=b;};
  
  void   id(const size_t id){id_ = id;};
  size_t id()const{return id_;};
  
  double unitCost()const{return unitCost_;};
  void   scaleCost(const double s = 1.0E-6){unitCost_ *= s;};
  double capacity()const{return cap_;};
  
  size_t bus()const{return bus_;};
  friend istream& operator>>(istream& fin, generator& g)
  {
    fin >> g.id_ >> g.bus_ >> g.cap_ >> g.unitCost_; 
    g.scaleCost(1.0E-6);
    return fin;
  }
  friend ostream& operator<<(ostream& os, generator& g)
  {
    os << g.id_  << ":"
       << g.bus_ << " "
       << g.cap_ << " "
       << g.unitCost_; 
    return os;
  }
};
#endif
