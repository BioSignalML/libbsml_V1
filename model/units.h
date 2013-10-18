#ifndef _BSML_UNITS_H
#define _BSML_UNITS_H

#include "rdf.h"
#include "utility.h"


namespace bsml {

  class Unit : public rdf::Resource
  /*=============================*/
  {
   public:
    Unit(void) ;
    Unit(const std::string &unit) ;
    } ;

  namespace unit {
    static std::string NS("http://www.sbpax.org/uome/list.owl#") ;
    static rdf::Uri uri(NS) ;
    static Unit Millivolt(NS + "Millivolt") ;

    static strlist list {
      "Millivolt"
      } ;
    } ;


  Unit get_units(const std::string &unit) ;

  } ;

#endif
