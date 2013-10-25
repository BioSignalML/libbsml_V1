#include <string>

#include "rdf/rdf.h"
#include "model/recording.h"
#include "model/signal.h"
#include "model/units.h"


using namespace bsml ;


Signal::Signal(void)
/*----------------*/
{ }

Signal::Signal(const std::string &uri, const Unit &unit, double rate)
/*-----------------------------------------------------------------*/
: AbstractObject(BSML::Signal, uri), recording_(NULL), unit_(unit), rate_(rate), clock_(NULL)
{
  rdfmap.push_back(new rdf::Mapping<Unit>(BSML::units, &unit_)) ;
  literal_rate = rdf::Literal(rate_) ;
  rdfmap.push_back(new rdf::Mapping<rdf::Literal>(BSML::rate, &literal_rate)) ;
  }

Signal::Signal(const std::string &uri, const Unit &unit, Clock *clock)
/*------------------------------------------------------------------*/
: AbstractObject(BSML::Signal, uri), recording_(NULL), unit_(unit), rate_(0.0), clock_(clock)
//: Signal(uri, Unit(unit))  // C++11
{
  rdfmap.push_back(new rdf::Mapping<Unit>(BSML::units, &unit_)) ;
  rdfmap.push_back(new rdf::Mapping<rdf::Resource>(BSML::clock, clock->get_resource())) ;
  }

void Signal::set_recording(bsml::Recording *recording)
/*--------------------------------------------------*/
{
  // Error if signal already attached to a recording 
  recording_ = recording ;
  rdfmap.push_back(new rdf::Mapping<rdf::Resource>(BSML::recording, recording->get_resource())) ;
  }


/*
  metaclass = BSML.Signal     #: :attr:`.BSML.Signal`

  attributes = ['recording', 'units', 'transducer', 'filter', '_rate',  '_period', 'clock',
                'minFrequency', 'maxFrequency', 'minValue', 'maxValue',
                'index', 'signaltype', 'offset', '_duration',
               ]
  '''Generic attributes of a Signal.'''

  mapping = { 'recording':    PropertyMap(BSML.recording, to_rdf=PropertyMap.get_uri),
              'units':        PropertyMap(BSML.units, to_rdf=PropertyMap.get_uri),
              'sensor':       PropertyMap(BSML.sensor),
              'filter':       PropertyMap(BSML.preFilter),
              '_rate':        PropertyMap(BSML.rate, XSD.double),
              '_period':      PropertyMap(BSML.period, XSD.double),
              'clock':        PropertyMap(BSML.clock, to_rdf=PropertyMap.get_uri, subelement=True),
              'minFrequency': PropertyMap(BSML.minFrequency, XSD.double),
              'maxFrequency': PropertyMap(BSML.maxFrequency, XSD.double),
              'minValue':     PropertyMap(BSML.minValue, XSD.double),
              'maxValue':     PropertyMap(BSML.maxValue, XSD.double),
              'dataBits':     PropertyMap(BSML.dataBits, XSD.integer),
              'signaltype':   PropertyMap(BSML.signalType),
              'offset':       PropertyMap(BSML.offset, XSD.dayTimeDuration,
                                          utils.seconds_to_isoduration,
                                          utils.isoduration_to_seconds),
              '_duration':    PropertyMap(DCT.extent, XSD.dayTimeDuration,
                                          utils.seconds_to_isoduration,
                                          utils.isoduration_to_seconds),
            }

*/
