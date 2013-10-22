#ifndef _BSML_SEGMENT_H
#define _BSML_SEGMENT_H


#include "model/bsml.h"
#include "model/object.h"
#include "rdf/rdfmap.h"


namespace bsml {

  class Segment : public bsml::AbstractObject
  /*======================================*/
  {
   public:
    Segment(const std::string &uri)
    /*---------------------------*/
    : bsml::AbstractObject(bsml::BSML::Segment, uri)
    {
//      rdfmap.push_back(rdf::Mapping()) ;

      }
    } ;

  } ;

/*

  metaclass = BSML.Segment  #: :attr:`.BSML.Segment`

  attributes = [ 'source', 'time' ]

  mapping = { 'source': PropertyMap(DCT.source, to_rdf=PropertyMap.get_uri),
              'time':   PropertyMap(BSML.time, subelement=True),
            }

*/

#endif
