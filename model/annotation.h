#ifndef _BSML_ANNOTATION_H
#define _BSML_ANNOTATION_H


#include "bsml.h"

#include "object.h"
#include "rdfmap.h"


namespace bsml {

  class Annotation : public bsml::AbstractObject
  /*======================================*/
  {
   public:
    Annotation(const std::string &uri)
    : bsml::AbstractObject(bsml::BSML::Annotation, uri)
    {
//      rdfmap.push_back(rdf::Mapping()) ;

      }
    } ;

  } ;

/*

  metaclass = BSML.Annotation  #: :attr:`.BSML.Annotation

  '''Attributes of an Annotation.'''
  attributes = [ 'about', 'comment', 'tags' ]

  mapping = { 'about':   PropertyMap(DCT.subject, to_rdf=PropertyMap.get_uri),
              'comment': PropertyMap(RDFS.comment),
              'tags':    PropertyMap(BSML.tag, functional=False),
            }

*/

#endif
