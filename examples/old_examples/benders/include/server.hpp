// ===========================================================================
// $Date: 2010-03-07 22:35:34 -0600 (Sun, 07 Mar 2010) $
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: server.hpp 677 2010-04-05 18:35:37Z xzhen $
// ===========================================================================
#ifndef SERVER_HH_
#define SERVER_HH_

#include <iterator>
#include <list>
#include <map>

using namespace::std;

// ============================================================
// class definition
// ============================================================
template<typename T>
class server{
  typedef typename list<T>::iterator Iter;
  typedef typename list<T>::const_iterator IterC;
private:
  list<T> data_;
  map<size_t, T*>   indxPt_;
  map<size_t, Iter> indxIt_;
public:
  server(){};
  server(const server<T>& sb);
  server<T>& operator=(const server<T>& sb);
  T*   add(const T& item, const size_t key);
  bool erase(const size_t key);
  T*   pick(const size_t key)const;
  T*   find(const T& item)const;
  size_t size()const{return data_.size();};
  void clear(){data_.clear(); indxPt_.clear(); indxIt_.clear();};
};

// ============================================================
// class implementation
// ============================================================
template<typename T>
server<T>::server(const server<T>& sb)
{
  data_ = sb.data_;
  indxIt_.clear();
  indxPt_.clear();
  typedef typename map<size_t, Iter>::const_iterator cit;
  IterC sit_begin  = sb.data_.begin();
  
  cit   it_begin   = sb.indxIt_.begin();
  cit   it_end     = sb.indxIt_.end();
  cit   it         = it_begin;
  
  while(it!=it_end){
    Iter    jt = data_.begin();
    size_t key = it->first;	// a key in the map
    IterC  sit = it->second;	// corresponding data iter for the key
    advance(jt, distance(sit_begin, sit));
    indxIt_[key] = jt;
    indxPt_[key] = &(*jt);
    ++it;
  }
}

template<typename T>
server<T>& server<T>::operator=(const server<T>& sb)
{
  if(this != &sb){
    data_ = sb.data_;
    indxIt_.clear();
    indxPt_.clear();
    typedef typename map<size_t, Iter>::const_iterator cit;
    
    IterC sit_begin  = sb.data_.begin();
    
    cit   it_begin   = sb.indxIt_.begin();
    cit   it_end     = sb.indxIt_.end();
    cit   it         = it_begin;
    
    while(it!=it_end){
      Iter    jt = data_.begin();
      size_t key = it->first;	// a key in the map
      IterC  sit = it->second;	// corresponding data iter for the key
      advance(jt, distance(sit_begin, sit));
      indxIt_[key] = jt;
      indxPt_[key] = &(*jt);
      ++it;
    }
  }
  return *this;
}

template<typename T>
T* server<T>::add(const T& item, const size_t key)
{
  if(indxPt_.find(key) == indxPt_.end()){ // unique key
    data_.push_back(item);
    Iter it = data_.end();
    --it;
    T* pt = &(*it);
    indxPt_[key] = pt;
    indxIt_[key] = it;
    return pt;
  }else return NULL;		// duplicated key
}

template<typename T>
T* server<T>::pick(const size_t key)const
{
  typename map<size_t, T*>::const_iterator it = indxPt_.find(key);
  
  if(it != indxPt_.end()){	// existing key
    return it->second;
  }else return NULL;		// non-existing key
}

template<typename T>
T* server<T>::find(const T& item)const
{
  typename map<size_t, T*>::const_iterator ita = indxPt_.begin();
  typename map<size_t, T*>::const_iterator itz = indxPt_.end();
  
  while(ita != itz){
    if(*(ita->second) == item) return ita->second;
    ++ita;
  }
  return NULL;
}

template<typename T>
bool server<T>::erase(const size_t key)
{
  typename map<size_t, Iter>::iterator it = indxIt_.find(key);
  if(it != indxIt_.end()){	// existing key
    data_.erase(it->second);
    indxIt_.erase(key);
    indxPt_.erase(key);
    return true;
  }else return false;
}

#endif
