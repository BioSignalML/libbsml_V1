#ifndef _BSML_RDFNAMES_H
#define _BSML_RDFNAMES_H

#include "rdf.h"


namespace rdf
/*=========*/
{

  namespace RDF
  /*==========*/
  {
    static std::string NS("http://www.w3.org/1999/02/22-rdf-syntax-ns#") ;
    static rdf::Resource type(NS + "type") ;
    } ;

  namespace RDFS
  /*==========*/
  {
    static std::string NS("http://www.w3.org/2000/01/rdf-schema#") ;
    static rdf::Resource comment(NS + "comment") ;
    static rdf::Resource label(NS + "label") ;
    } ;

  namespace DCT
  /*=========*/
  {
    static std::string NS("http://purl.org/dc/terms/") ;
    static rdf::Resource created(NS + "created") ;
    static rdf::Resource creator(NS + "creator") ;
    static rdf::Resource description(NS + "description") ;
    static rdf::Resource extent(NS + "extent") ;
    static rdf::Resource format(NS + "format") ;
    static rdf::Resource source(NS + "source") ;
    static rdf::Resource subject(NS + "subject") ;
    } ;

/*

XSD.dayTimeDuration
XSD.dateTime
PRV.precededBy
TL.timeline
PROV.wasGeneratedBy

  'xsd':  'http://www.w3.org/2001/XMLSchema#'

  'owl':  'http://www.w3.org/2002/07/owl#'
  'time': 'http://www.w3.org/2006/time#'
  'tl':   'http://purl.org/NET/c4dm/timeline.owl#'

  'uome': 'http://www.sbpax.org/uome/list.owl#'

  'prv':  'http://purl.org/net/provenance/ns#'
  'prov': 'http://www.w3.org/ns/prov#'

*/

  } ;

#endif
