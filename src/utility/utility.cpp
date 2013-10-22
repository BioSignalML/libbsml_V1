#include <string>
#include <list>
#include <algorithm>

#include "utility.h"


using namespace bsml ;


strlist::strlist(const std::string &source, const std::string &split)
/*-----------------------------------------------------------------*/
: std::list<std::string>()
{
  size_t l = split.length() ;
  size_t lp = 0 ;
  size_t p = source.find(split, lp) ;
  while (p != std::string::npos) {
    this->push_back(source.substr(lp, p-lp)) ;
    lp = p + l ;
    p = source.find(split, lp) ;
    }
  this->push_back(source.substr(lp, p)) ;
  }


strlist::strlist(const strlist &source, std::string (*mapping)(const std::string &))
/*--------------------------------------------------------------------------------*/
: std::list<std::string>()
{
  for (auto &s: source) this->push_back(mapping(s)) ;
  }


strlist::strlist(std::initializer_list<value_type> il)
/*--------------------------------------------------*/
: std::list<std::string>(il)
{
  }


bool strlist::contains(const std::string &member)
/*---------------------------------------------*/
{
  return std::count(this->begin(), this->end(), member) > 0 ;
  }


std::string strlist::join(const std::string &joiner)
/*------------------------------------------------*/
{
  std::string r = "" ;
  for (std::list<std::string>::iterator s = this->begin() ;
         s != this->end() ;  ++s) {
    if (s != this->begin()) r += joiner ;
    r += *s ;
    }
  return r ;
  }


#ifdef TEST_UTILITY

#include <iostream>

int main(void)
/*==========*/
{
  strlist s("12,1,2,3", ",") ;
  strlist t{"a2", "1" , "2", "Z"} ;

  for (std::list<std::string>::iterator x = s.begin() ;  x != s.end() ;  ++x) {
    std::cout << *x << std::endl ;
    }

  std::cout << s.join("**") << std::endl ;
  std::cout << t.join("+") << std::endl ;
  }

#endif
