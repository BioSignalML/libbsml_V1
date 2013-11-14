#ifndef _BSML_DATETIME_H
#define _BSML_DATETIME_H

#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "config.h"
#include "rdf/rdf.h"


namespace bsml {

  class BSML_EXPORT Datetime : public rdf::Literal
  /*============================================*/
  {
   private:
    boost::posix_time::ptime datetime ;
   public:
    Datetime(void) ;
    std::string as_iso_string(void) ;
    } ;

  class BSML_EXPORT Duration : public rdf::Literal
  /*============================================*/
  {
   private:
    boost::posix_time::time_duration duration ;
   public:
    Duration(void) ;
    } ;

  } ;

#endif
