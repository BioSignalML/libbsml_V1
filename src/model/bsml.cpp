#include <string>

#include "model/bsml.h"
#include "rdf/rdf.h"


using namespace bsml ;


rdf::Literal Format::HDF5("application/x-bsml+hdf5") ;


std::string BSML::NS("http://www.biosignalml.org/ontologies/2011/04/biosignalml#") ;
rdf::Uri BSML::uri(BSML::NS) ;
rdf::Resource BSML::Recording(BSML::NS + "Recording") ;
rdf::Resource BSML::Signal(BSML::NS + "Signal") ;
rdf::Resource BSML::SampleClock(BSML::NS + "SampleClock") ;
rdf::Resource BSML::Segment(BSML::NS + "Segment") ;
rdf::Resource BSML::Annotation(BSML::NS + "Annotation") ;
rdf::Resource BSML::Event(BSML::NS + "Event") ;

rdf::Resource BSML::clock(BSML::NS + "clock") ;
rdf::Resource BSML::dataBits(BSML::NS + "dataBits") ;
rdf::Resource BSML::dataset(BSML::NS + "dataset") ;
rdf::Resource BSML::digest(BSML::NS + "digest") ;
rdf::Resource BSML::eventType(BSML::NS + "eventType") ;
rdf::Resource BSML::maxFrequency(BSML::NS + "maxFrequency") ;
rdf::Resource BSML::maxValue(BSML::NS + "maxValue") ;
rdf::Resource BSML::minFrequency(BSML::NS + "minFrequency") ;
rdf::Resource BSML::minValue(BSML::NS + "minValue") ;
rdf::Resource BSML::offset(BSML::NS + "offset") ;
rdf::Resource BSML::period(BSML::NS + "period") ;
rdf::Resource BSML::preFilter(BSML::NS + "preFilter") ;
rdf::Resource BSML::rate(BSML::NS + "rate") ;
rdf::Resource BSML::recording(BSML::NS + "recording") ;
rdf::Resource BSML::sensor(BSML::NS + "sensor") ;
rdf::Resource BSML::signalType(BSML::NS + "signalType") ;
rdf::Resource BSML::tag(BSML::NS + "tag") ;
rdf::Resource BSML::time(BSML::NS + "time") ;
rdf::Resource BSML::units(BSML::NS + "units") ;
