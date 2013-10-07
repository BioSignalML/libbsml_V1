#ifndef _BSML_DATETIME_H
#define _BSML_DATETIME_H

#include "rdf.h"
#include "rdfnames.h"

namespace bsml {

  class Datetime : public rdf::Literal
  /*================================*/
  {

   public:
    Datetime(void)
    /*----------*/
    : rdf::Literal("", rdf::XSD::dateTime)
    {
      }

    } ;

  } ;

#endif
