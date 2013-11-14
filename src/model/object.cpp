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
  rdfmap.append(rdf::RDF::type, &metatype) ;

  rdfmap.append(rdf::RDFS::label, &label_) ;
  rdfmap.append(rdf::RDFS::comment, &comment_) ;
  rdfmap.append(rdf::DCT::description, &description_) ;

  rdfmap.append(rdf::PRV::precededBy, &precededby_) ;
  rdfmap.append(rdf::DCT::creator, &creator_) ;
  rdfmap.append(rdf::DCT::created, &created_) ;

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

const std::string AbstractObject::get_uri_as_string(void) const
/*-----------------------------------------------------------*/
{
  return resource.as_string() ;
  }

void AbstractObject::to_rdf(rdf::Graph &graph)
/*------------------------------------------*/
{
  rdfmap.to_rdf(graph, resource) ;
  }
