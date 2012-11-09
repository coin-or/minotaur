#ifndef POWERLINE_HH_
#define POWERLINE_HH_

#include <iostream>
using namespace std;

class powerline{
private:
  size_t id_;
  size_t sendBus_, recvBus_;
  double cap_;
  double x_;
  double v_;
  double cost_;
  short  status_;		// 0: cand off; 1: bult, 2: cand on;
public:
  powerline(){};
  powerline(const size_t id, const size_t ba, const size_t bb, const bool s)
  {id_=id;sendBus_=ba;recvBus_=bb;status_=s;};
  
  void   id(const size_t id){id_ = id;};
  size_t id()const{return id_;};
  
  void   on() {if(status_!=1) status_ = 2;};
  void   off(){if(status_!=1) status_ = 0;};
  short  status()const{return status_;};
  
  double capacity()const{return cap_;};
  double x()const{return x_;};
  double v()const{return v_;};
  size_t from()const{return sendBus_;};
  size_t to()const{return recvBus_;};
  double cost()const{return cost_;};
  
  friend istream& operator>>(istream& fin, powerline& l)
  {
    fin >> l.id_ >> l.sendBus_ >> l.recvBus_ >> l.status_ 
	>> l.cap_ >> l.x_ >> l.v_ >> l.cost_; 
    return fin;
  }
  friend ostream& operator<<(ostream& os, powerline&l)
  {
    os << l.id_      << ":"
       << l.sendBus_ << " "
       << l.recvBus_ << " "
       << l.status_; 
    return os;
  }
};
#endif
