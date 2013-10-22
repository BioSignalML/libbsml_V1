#ifndef _BSML_DATETIME_H
#define _BSML_DATETIME_H

#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "rdf/rdf.h"


namespace bsml {

  class Datetime : public rdf::Literal
  /*================================*/
  {
   private:
    boost::posix_time::ptime datetime ;
   public:
    Datetime(void) ;
    std::string as_iso_string(void) ;
    } ;

  class Duration : public rdf::Literal
  /*================================*/
  {
   private:
    boost::posix_time::time_duration duration ;
   public:
    Duration(void) ;
    } ;

  } ;

#endif
