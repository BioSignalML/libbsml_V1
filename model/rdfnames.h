#ifndef _BSML_RDFNAMES_H
#define _BSML_RDFNAMES_H

#include "rdf.h"


namespace rdf
/*=========*/
{

  namespace RDF
  /*==========*/
  {
    static std::string NAMESPACE = "http://www.w3.org/1999/02/22-rdf-syntax-ns#" ;

    static rdf::Resource type = rdf::Resource(NAMESPACE + "type") ;
    } ;


  namespace RDFS
  /*==========*/
  {
    static std::string NAMESPACE = "http://www.w3.org/2000/01/rdf-schema#" ;

    static rdf::Resource label = rdf::Resource(NAMESPACE + "label") ;
    } ;


//  'xsd':  'http://www.w3.org/2001/XMLSchema#',
//  'owl':  'http://www.w3.org/2002/07/owl#',
//  'dct':  'http://purl.org/dc/terms/',
//  'time': 'http://www.w3.org/2006/time#',
//  'tl':   'http://purl.org/NET/c4dm/timeline.owl#',
//  'uome': 'http://www.sbpax.org/uome/list.owl#',
//  'prv':  'http://purl.org/net/provenance/ns#',
//  'prov': 'http://www.w3.org/ns/prov#',


  } ;

#endif
