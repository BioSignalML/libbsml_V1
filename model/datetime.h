#ifndef _BSML_DATETIME_H
#define _BSML_DATETIME_H

#include "rdf.h"


namespace bsml {

  class Datetime : public rdf::Literal
  /*================================*/
  {
   public:
    Datetime(void) ;
    } ;

  class Duration : public rdf::Literal
  /*================================*/
  {
   public:
    Duration(void) ;
    } ;

  } ;

#endif
