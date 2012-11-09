// ===========================================================================
// $Date: 2010-03-07 22:35:34 -0600 (Sun, 07 Mar 2010) $
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: agent.hpp 677 2010-04-05 18:35:37Z xzhen $
// ===========================================================================
#ifndef AGENT_HH_
#define AGENT_HH_
// ========================================
// Agent With Storage
// ========================================
#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include "server.hpp"
using namespace std;

template<typename T> class agent;
template<typename T> ostream& operator<<(ostream &os,const agent<T> &a);
// ============================================================
// class definition
// ============================================================
template<typename T>
class agent{
private:
  size_t seed_;
  size_t coid_;
  server<T> archive_;
  list<size_t> lok_;		// list of keys
  list<T*> lop_;		// list of pointers
  size_t next(){++coid_;return coid_-1;}
  void   reset(){coid_ = seed_;};
public:
  
  typedef typename list<T*>::const_iterator const_iterator;
  typedef typename list<T*>::iterator iterator;
  
  agent(){seed_ = 1000; coid_ = 1000;};
  agent(const size_t seed){seed_=seed; coid_=seed;};
  agent(const agent<T>& ab);
  
  size_t size()const{return lok_.size();};
  
  const_iterator begin()const{return lop_.begin();};
  const_iterator end()const{return lop_.end();};
  
  agent<T>& operator=(const agent<T>& ab);
  
  T*     add(const T& item);
  T*     add(const T& item, size_t& key);
  bool   erase(const size_t& key);
  T*     pick(const size_t& key)const;
  T*     find(const T& item)const;
  list<size_t> keys()const{return lok_;};
  vector<size_t> vkey()const{vector<size_t> v(lok_.begin(),lok_.end());return v;};
  list<T*>     lpt()const{return lop_;};
  vector<T*>   vpt()const{vector<T*> v(lop_.begin(), lop_.end()); return v;};
  void   clear(const size_t seed=1000)
  {archive_.clear(); lok_.clear();lop_.clear(); seed_ = seed; coid_ = seed_;};
  
  friend ostream & operator<< <>(ostream &os, const agent<T>& a);
};

// ============================================================
// class implementation
// ============================================================


template<typename T>
agent<T>::agent(const agent<T>& ab)
{
  seed_    = ab.seed_;
  coid_    = ab.coid_;
  archive_ = ab.archive_;
  lok_     = ab.lok_;		// list of keys
  lop_.clear();
  for(list<size_t>::iterator it=lok_.begin();it!=lok_.end();++it){
    size_t key = *it;
    lop_.push_back(archive_.pick(key));	// list of pointers
  }
}

template<typename T>
agent<T>& agent<T>::operator=(const agent<T>& ab)
{
  if(this != &ab){
    seed_    = ab.seed_;
    coid_    = ab.coid_;
    archive_ = ab.archive_;
    lok_     = ab.lok_;		// list of keys
    lop_.clear();
    for(list<size_t>::iterator it=lok_.begin();it!=lok_.end();++it){
      size_t key = *it;
      lop_.push_back(archive_.pick(key));	// list of pointers
    }
  }
  return *this;
}

template<typename T>
T* agent<T>::add(const T& item)
{
  size_t key = next();
  T* pt = archive_.add(item, key); 
  if(pt!=NULL) {
    lok_.push_back(key);
    lop_.push_back(pt);
  }
  return pt;
}

template<typename T>
T* agent<T>::add(const T& item, size_t& key)
{
  key = next();
  T* pt = archive_.add(item, key); 
  if(pt!=NULL) {
    lok_.push_back(key);
    lop_.push_back(pt);
  }
  return pt;
}

template<typename T>
T* agent<T>::pick(const size_t& key)const
{
  return archive_.pick(key);
}

template<typename T>
T* agent<T>::find(const T& item)const
{
  return archive_.find(item);
}

template<typename T>
bool agent<T>::erase(const size_t& key)
{
  bool stat = false;
  list<size_t>::iterator it = find(lok_.begin(), lok_.end(), key);
  typename list<T*>::iterator jt = lop_.begin();
  if(it!=lok_.end()){
    stat = true;
    advance(jt, distance(lok_.begin(), it));
    lok_.erase(it);
    lop_.erase(jt);
    archive_.erase(key);
  }
  return  stat;
}

template<typename T>
ostream & operator<<(ostream &os, const agent<T>& a)
{
  typename list<T*>::const_iterator it  = a.lop_.begin();
  typename list<T*>::const_iterator itz = a.lop_.end();
  while(it!=itz){
    os << **it;
    ++it;
  }
  return os;
}
#endif
