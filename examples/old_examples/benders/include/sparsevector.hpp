// ========================================================
// this is a class for the sparse vectors, 
// ========================================================

#ifndef SPARSEVECTOR_HH_
#define SPARSEVECTOR_HH_

#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <assert.h>
#include <cmath>
#include "type.hpp"

using namespace std;
using Type::double;
using Type::EPSILON;
class sparseVector{
private:
  size_t       n_;		// the size of the vector
  map<size_t, double> entry_;	// index -> value
public:
  sparseVector(){};
  sparseVector(const size_t n) :n_(n){};
  sparseVector(const size_t n, 
	       const list<size_t> &idx, const list<double> &val);
  sparseVector(const size_t n, 
	       const vector<size_t> &idx, const vector<double> &val);
  sparseVector(const size_t n, const map<size_t, double>& entry) 
    :n_(n), entry_(entry){};
  
  size_t ne()  const{return entry_.size();};
  size_t size()const{return n_;};
  
  void   add(const size_t i, const double v);
  size_t read(const string& str, const bool sparse = true); // default sparse
  
  void   extract(vector<size_t> &idx, vector<double> &val)const;
  // ========================================
  // we suppsed we never add duplicated entry
  
  sparseVector  subv(const size_t k); 
  // ============================================================
  // a sub sparse vector, without the original first k locations
  
  double        operator[](const size_t i);
  sparseVector& operator+=(const sparseVector& sv);
  sparseVector& operator-=(const sparseVector& sv);
  sparseVector& operator*=(const double s);
  sparseVector  operator+ (const sparseVector& sv);
  sparseVector  operator- (const sparseVector& sv);
  sparseVector  operator* (const double s);
  
  friend sparseVector operator-(const sparseVector& sv);
  friend ostream& operator<<(ostream& os, const sparseVector& sv);
};

#endif
