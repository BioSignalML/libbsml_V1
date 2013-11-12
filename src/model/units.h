#ifndef _BSML_UNITS_H
#define _BSML_UNITS_H

#include "rdf/rdf.h"
#include "utility/utility.h"


namespace bsml {

  class Unit : public rdf::Resource
  /*=============================*/
  {
   public:
    Unit(void) ;
    Unit(const std::string &unit) ;
    Unit(const std::string &prefix, const std::string &unit) ;

    static std::string NS ;
    static rdf::Uri uri ;
    static Unit Second ;
    static Unit Minute ;
    static Unit Millivolt ;

    static strlist list ;

    static Unit get_unit(const std::string &unit) ;
    } ;



  } ;

#endif
