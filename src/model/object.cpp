#include <string>
#include <list>

#include "model/bsml.h"
#include "model/object.h"
#include "rdf/rdfnames.h"
              

using namespace bsml ;


AbstractObject::AbstractObject(void)
/*--------------------------------*/
{ }

AbstractObject::AbstractObject(const rdf::Resource &type, const std::string &uri)
/*-----------------------------------------------------------------------------*/
: resource(rdf::Resource(uri)), metatype(type), rdfmap()
{
  rdfmap.push_back(new rdf::Mapping<rdf::Resource>(rdf::RDF::type, &metatype)) ;

  rdfmap.push_back(new rdf::Mapping<std::string>(rdf::RDFS::label, &label_)) ;
  rdfmap.push_back(new rdf::Mapping<std::string>(rdf::RDFS::comment, &comment_)) ;
  rdfmap.push_back(new rdf::Mapping<std::string>(rdf::DCT::description, &description_)) ;

  rdfmap.push_back(new rdf::Mapping<rdf::Resource>(rdf::PRV::precededBy, &precededby_)) ;
  rdfmap.push_back(new rdf::Mapping<rdf::Node>(rdf::DCT::creator, &creator_)) ;
  rdfmap.push_back(new rdf::Mapping<Datetime>(rdf::DCT::created, &created_)) ;

/*
              set_predecessor(const rdf::Resource &predecessor)

              set_creator(const rdf::Resource &creator)
              set_creator(const std::string &creator)

              set_created(const Datetime &created)
*/

  }

AbstractObject::~AbstractObject(void)
/*---------------------------------*/
{
  for (std::list<rdf::MapBase *>::iterator map = rdfmap.begin() ; map != rdfmap.end(); ++map)
    delete *map ;
  }

void AbstractObject::set_label(const std::string &label)
/*----------------------------------------------------*/
{
  label_ = label ;
  }

void AbstractObject::set_comment(const std::string &comment)
/*--------------------------------------------------------*/
{
  comment_ = comment ;
  }

void AbstractObject::set_description(const std::string &description)
/*----------------------------------------------------------------*/
{
  description_ = description ;
  }

const rdf::Resource *AbstractObject::get_resource(void)
/*---------------------------------------------------*/
{
  return &resource ;
  }

const std::string AbstractObject::get_uri_as_string(void)
/*-----------------------------------------------------*/
{
  return resource.as_string() ;
  }

void AbstractObject::to_rdf(const rdf::Graph &graph)
/*------------------------------------------------*/
{
  for (std::list<rdf::MapBase *>::iterator map = rdfmap.begin() ; map != rdfmap.end(); ++map)
    (*map)->to_rdf(graph, resource) ;
  }
