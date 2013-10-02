#ifndef _BSML_EVENT_H
#define _BSML_EVENT_H


#include "bsml.h"

#include "object.h"
#include "rdfmap.h"


namespace bsml {

  class Event : public bsml::AbstractObject
  /*======================================*/
  {
   public:
    Event(const std::string &uri)
    : bsml::AbstractObject(bsml::BSML::Event, uri)
    {
//      rdfmap.push_back(rdf::Mapping()) ;

      }
    } ;

  } ;

/*

  metaclass = BSML.Event      #: :attr:`.BSML.Event`

  attributes = ['eventtype', 'time', 'recording' ]
  '''Generic attributes of an Event.'''

  mapping = { 'recording': PropertyMap(BSML.recording, to_rdf=PropertyMap.get_uri),
              'eventtype': PropertyMap(BSML.eventType),
              'time':      PropertyMap(BSML.time, subelement=True),
            }

*/

#endif
