#ifndef _BSML_BSML_H
#define _BSML_BSML_H

#include <string>

#include "rdf.h"


namespace bsml {

  namespace BSML {
    static std::string NAMESPACE   = "http://www.biosignalml.org/ontologies/2011/04/biosignalml#" ;
    static rdf::Resource Signal    = rdf::Resource(NAMESPACE + "Signal") ;
    static rdf::Resource Recording = rdf::Resource(NAMESPACE + "Recording") ;

    } ;


  } ;

#endif
