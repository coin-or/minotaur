#include "genobj.hpp"

bool trim(string& str)
{
  bool stat = false;
  size_t  p = 0;
  p = str.find_first_not_of(" \t\n");
  if(p > 0) {
    str.erase(0,p); stat = true;
  }
  p = str.find_last_not_of(" \t\n");
  if(p < str.length()){
    str.erase(p+1); stat = true;
  }
  return stat;
}
