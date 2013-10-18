#ifndef _BSML_UTILITY_H
#define _BSML_UTILITY_H

#include <string>
#include <list>
#include <map>


namespace bsml {

  typedef std::map<std::string, std::string> dict ;

  class strlist : public std::list<std::string>
  /*=========================================*/
  {
   public:
    strlist(const std::string &source, const std::string &split) ;
    strlist(const strlist &source, std::string (*mapping)(const std::string &)) ;
    strlist(std::initializer_list<value_type> il) ;
    bool contains(const std::string &member) ;
    std::string join(const std::string &joiner) ;
    } ;

  } ;

#endif
