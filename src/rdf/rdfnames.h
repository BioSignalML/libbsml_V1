#ifndef _BSML_RDFNAMES_H
#define _BSML_RDFNAMES_H

#include "rdf/rdf.h"


namespace rdf {

  class RDF
  /*=======*/
  {
   public:
    static std::string NS ;
    static rdf::Uri uri ;
    static rdf::Resource type ;
    } ;

  class RDFS
  /*=======*/
  {
   public:
    static std::string NS ;
    static rdf::Uri uri ;
    static rdf::Resource comment ;
    static rdf::Resource label ;
    } ;

  class DCT
  /*======*/
  {
   public:
    static std::string NS ;
    static rdf::Uri uri ;
    static rdf::Resource created ;
    static rdf::Resource creator ;
    static rdf::Resource description ;
    static rdf::Resource extent ;
    static rdf::Resource format ;
    static rdf::Resource source ;
    static rdf::Resource subject ;
    } ;

  class PRV
  /*======*/
  {
   public:
    static std::string NS ;
    static rdf::Uri uri ;
    static rdf::Resource precededBy ;
    } ;

  class XSD
  /*======*/
  {
   public:
    static std::string NS ;
    static rdf::Uri uri ;
    static rdf::Resource XSD_integer ;
    static rdf::Resource XSD_double ;
    static rdf::Resource XSD_dateTime ;
    static rdf::Resource XSD_dayTimeDuration ;
    } ;

/*

TL.timeline
PROV.wasGeneratedBy

  'owl':  'http://www.w3.org/2002/07/owl#'
  'time': 'http://www.w3.org/2006/time#'
  'tl':   'http://purl.org/NET/c4dm/timeline.owl#'

  'uome': 'http://www.sbpax.org/uome/list.owl#'

  'prov': 'http://www.w3.org/ns/prov#'

*/

  } ;

#endif
