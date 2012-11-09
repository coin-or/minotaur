#include "sparsevector.hpp"

sparseVector::sparseVector(const size_t n, 
			   const list<size_t> &idx, 
			   const list<double> &val)
  :n_(n)
{
  list<size_t>::const_iterator iti = idx.begin();
  list<double>::const_iterator itv = val.begin();
  for(size_t i=0;i<idx.size();++i) entry_[*iti++] = *itv++;
}

sparseVector::sparseVector(const size_t n, 
			   const vector<size_t> &idx, 
			   const vector<double> &val)
  :n_(n)
{
  for(size_t i=0;i<idx.size();++i) {
    entry_[idx[i]] = val[i];
  }
}

void sparseVector::add(const size_t i, const double v)
{
  assert(i < n_);
  map<size_t, double>::iterator it = entry_.find(i);
  if(it == entry_.end()){
    entry_[i] = v;
  }else{
    it->second += v;
  }
}

size_t sparseVector::read(const string& str, const bool sparse)
{
  
  entry_.clear();
  istringstream iss(str);
  double val;
  size_t i,k=0;
  if(sparse){
    while(iss >> i >> val){
      if(fabs(val) > EPSILON){
	this->add(i,val);
	++k;
      }
    }
  }else{
    i = 0;
    while(iss >> val){
      if(fabs(val) > EPSILON){
	this->add(i,val);
	++k;
      };
      ++i;
    }
  }
  return k;
}

void sparseVector::extract(vector<size_t> &idx, vector<double> &val)const
{
  size_t n = entry_.size();
  idx.clear(); idx.resize(n);
  val.clear(); val.resize(n);
  
  map<size_t, double>::const_iterator it = entry_.begin();
  size_t i = 0;
  while(it!= entry_.end()){
    idx[i] = it->first;
    val[i] = it->second;
    ++i;
    ++it;
  }
}

sparseVector sparseVector::subv(const size_t k)
{
  map<size_t, double> ne;	// map to save the new sparse vector
  map<size_t, double>::iterator it = entry_.begin();
  while(it!=entry_.end()){
    if(it->first >= k){
      ne[it->first - k] = it->second;
    }
    ++it;
  }
  sparseVector subsv(this->size()-k, ne);
  return subsv;
}

double sparseVector::operator[](const size_t i)
{
  assert(i < n_);
  map<size_t, double>::iterator it = entry_.find(i);
  if(it == entry_.end()){
    return 0;
  }else{
    return it->second;
  }
}

sparseVector& sparseVector::operator-=(const sparseVector& sv)
{
  map<size_t, double>::const_iterator it = sv.entry_.begin();
  while(it!=sv.entry_.end()){
    this->add(it->first, -it->second);
    ++it;
  }
  return *this;
}

sparseVector& sparseVector::operator*=(const double s)
{
  map<size_t, double>::iterator it = entry_.begin();
  while(it!=entry_.end()){
    it->second *= s;
    ++it;
  }
  return *this;
}

sparseVector& sparseVector::operator+=(const sparseVector& sv)
{
  map<size_t, double>::const_iterator it = sv.entry_.begin();
  while(it!=sv.entry_.end()){
    this->add(it->first, it->second);
    ++it;
  }
  return *this;
}

sparseVector sparseVector::operator+(const sparseVector& sv)
{
  sparseVector vec(*this);
  map<size_t, double>::const_iterator it = sv.entry_.begin();
  while(it!=sv.entry_.end()){
    vec.add(it->first, it->second);
    ++it;
  }
  return vec;
}

sparseVector sparseVector::operator-(const sparseVector& sv)
{
  sparseVector vec(*this);
  map<size_t, double>::const_iterator it = sv.entry_.begin();
  while(it!=sv.entry_.end()){
    vec.add(it->first, -it->second);
    ++it;
  }
  return vec;
}

sparseVector sparseVector::operator*(const double s)
{
  sparseVector vec(*this);
  map<size_t, double>::iterator it = vec.entry_.begin();
  while(it!=vec.entry_.end()){
    it->second *= s;
    ++it;
  }
  return vec;
}


// here are the friend functions
ostream& operator<<(ostream& os, const sparseVector&sv)
{
  map<size_t, double>::const_iterator it = sv.entry_.begin();
  while(it!=sv.entry_.end()){
    os << '[' << it->first << ',' << it->second << ']' << " ";
    ++it;
  }
  return os;
}

sparseVector operator-(const sparseVector& sv)
{
  sparseVector vec(sv);
  map<size_t, double>::iterator it = vec.entry_.begin();
  while(it!=vec.entry_.end()){
    it->second *= -1;
    ++it;
  }
  return vec;
}
