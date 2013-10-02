#ifndef _BSML_BSML_H
#define _BSML_BSML_H

#include <string>

#include "rdf.h"

// This should be generated from the BSML Ontology...

namespace bsml {

  namespace BSML {
    static std::string NS("http://www.biosignalml.org/ontologies/2011/04/biosignalml#") ;
    static rdf::Resource Recording(NS + "Recording") ;
    static rdf::Resource Signal(NS + "Signal") ;
    static rdf::Resource Segment(NS + "Segment") ;
    static rdf::Resource Annotation(NS + "Annotation") ;
    static rdf::Resource Event(NS + "Event") ;

    static rdf::Resource clock(NS + "clock") ;
    static rdf::Resource dataBits(NS + "dataBits") ;
    static rdf::Resource dataset(NS + "dataset") ;
    static rdf::Resource digest(NS + "digest") ;
    static rdf::Resource eventType(NS + "eventType") ;
    static rdf::Resource maxFrequency(NS + "maxFrequency") ;
    static rdf::Resource maxValue(NS + "maxValue") ;
    static rdf::Resource minFrequency(NS + "minFrequency") ;
    static rdf::Resource minValue(NS + "minValue") ;
    static rdf::Resource offset(NS + "offset") ;
    static rdf::Resource period(NS + "period") ;
    static rdf::Resource preFilter(NS + "preFilter") ;
    static rdf::Resource rate(NS + "rate") ;
    static rdf::Resource recording(NS + "recording") ;
    static rdf::Resource sensor(NS + "sensor") ;
    static rdf::Resource signalType(NS + "signalType") ;
    static rdf::Resource tag(NS + "tag") ;
    static rdf::Resource time(NS + "time") ;
    static rdf::Resource units(NS + "units") ;
    } ;

  } ;

#endif
