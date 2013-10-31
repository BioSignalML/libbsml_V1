#ifndef _BSML_BSML_H
#define _BSML_BSML_H

#include <string>
#include <stdexcept>

#include "rdf/rdf.h"

// This should be generated from the BSML Ontology...

namespace bsml {

  class Exception : public std::runtime_error
  /*=======================================*/
  {
   public:
    Exception(const std::string &msg)
    : std::runtime_error(msg) { }
    } ;


  class Format
  /*=========*/
  {
   public:
    static rdf::Literal HDF5 ;
    } ;


  class BSML
  /*=======*/
  {
   public:
    static std::string NS ;
    static rdf::Uri uri ;
    static rdf::Resource Recording ;
    static rdf::Resource Signal ;
    static rdf::Resource SampleClock ;
    static rdf::Resource Segment ;
    static rdf::Resource Annotation ;
    static rdf::Resource Event ;

    static rdf::Resource clock ;
    static rdf::Resource dataBits ;
    static rdf::Resource dataset ;
    static rdf::Resource digest ;
    static rdf::Resource eventType ;
    static rdf::Resource maxFrequency ;
    static rdf::Resource maxValue ;
    static rdf::Resource minFrequency ;
    static rdf::Resource minValue ;
    static rdf::Resource offset ;
    static rdf::Resource period ;
    static rdf::Resource preFilter ;
    static rdf::Resource rate ;
    static rdf::Resource recording ;
    static rdf::Resource sensor ;
    static rdf::Resource signalType ;
    static rdf::Resource tag ;
    static rdf::Resource time ;
    static rdf::Resource units ;
    } ;

  } ;

#endif
