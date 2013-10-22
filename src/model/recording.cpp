#include <string>
#include <map>

#include "model/recording.h"
#include "model/signal.h"


using namespace bsml ;


Recording::Recording(void)
/*----------------------*/
{ }

Recording::Recording(const std::string &uri)
/*----------------------------------------*/
: AbstractObject(BSML::Recording, uri)
{
  }


void Recording::add_signal(bsml::Signal *signal)
/*--------------------------------------------*/
{
  const std::string uri = signal->get_uri_as_string() ;
  if (signals_.count(uri) == 0) {
    signals_.insert(std::pair<std::string, Signal *>(uri, signal)) ;
    signal->set_recording(this) ;
    }
  // Error if signal already attached to a recording 
  }

Signal Recording::new_signal(const std::string &uri, const Unit &unit)
/*------------------------------------------------------------------*/
{
  Signal signal = Signal(uri, unit) ;
  this->add_signal(&signal) ;
  return signal ;
  }

Signal Recording::new_signal(const std::string &uri, const std::string &unit)
/*-------------------------------------------------------------------------*/
{
  return new_signal(uri, Unit(unit)) ;
  }


void Recording::to_rdf(const rdf::Graph &graph)
/*-------------------------------------------*/
{
  std::map<std::string, Signal *>::iterator s ;
  AbstractObject::to_rdf(graph) ;
  for (s = signals_.begin() ;  s != signals_.end() ;  ++s) {
    s->second->to_rdf(graph) ;
    }
  }


/*

  metaclass = BSML.Recording  #: :attr:`.BSML.Recording`

  attributes = [ 'dataset', 'source', 'format', 'comment', 'investigation',
                 'starttime', 'duration', 'timeline', 'generatedBy'
               ]
  '''Generic attributes of a Recording.'''

  mapping = { 'format':        PropertyMap(DCT.format),
              'dataset':       PropertyMap(BSML.dataset),
              'source':        PropertyMap(DCT.source, functional=False),
              'investigation': PropertyMap(DCT.subject),
              'investigator':  PropertyMap(DCT.creator),
              'starttime':     PropertyMap(DCT.created, XSD.dateTime,
                                           utils.datetime_to_isoformat,
                                           utils.isoformat_to_datetime),
              'duration':      PropertyMap(DCT.extent, XSD.dayTimeDuration,
                                           utils.seconds_to_isoduration,
                                           utils.isoduration_to_seconds),
##            'digest':        PropertyMap(BSML.digest),
              'timeline':      PropertyMap(TL.timeline,
                                           to_rdf=PropertyMap.get_uri,
                                           from_rdf=_get_timeline, subelement=True),
              'generatedBy':   PropertyMap(PROV.wasGeneratedBy, to_rdf=PropertyMap.get_uri,
                                           subelement=True),
            }

  SignalClass = Signal       #: The class of Signals in the Recording
  EventClass  = Event        #: The class of Events in the Recording

*/
