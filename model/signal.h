#ifndef _BSML_SIGNAL_H
#define _BSML_SIGNAL_H


#include "bsml.h"

#include "object.h"
#include "rdfmap.h"


namespace bsml {

  class Signal : public bsml::AbstractObject
  /*======================================*/
  {
   public:
    Signal(const std::string &uri)
    : bsml::AbstractObject(bsml::BSML::Signal, uri)
    {
//      rdfmap.push_back(rdf::Mapping()) ;

      }
    } ;

  } ;

#endif
