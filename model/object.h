#ifndef _BSML_OBJECT_H
#define _BSML_OBJECT_H

#include <string>
#include <list>

#include "bsml.h"
#include "datetime.h"

#include "rdf.h"
#include "rdfmap.h"
#include "rdfnames.h"


namespace bsml {


  class AbstractObject
  /*================*/
  {
   private:
    const rdf::Resource resource ;
    const rdf::Resource metatype ;
    std::string label_ ;
    std::string comment_ ;
    std::string description_ ;
    rdf::Resource precededby_ ;
    rdf::Node creator_ ;       // Literal or Resource
    bsml::Datetime created_ ;

   protected:
    std::list<rdf::MapBase *> rdfmap ;

   public:
    AbstractObject(void)
    /*----------------*/
    { }

    AbstractObject(const rdf::Resource &type, const std::string &uri)
    /*-------------------------------------------------------------*/
    : resource(rdf::Resource(uri)), metatype(type), rdfmap()
    {
      rdfmap.push_back(new rdf::Mapping<rdf::Resource>(rdf::RDF::type, &metatype)) ;

      rdfmap.push_back(new rdf::Mapping<std::string>(rdf::RDFS::label, &label_)) ;
      rdfmap.push_back(new rdf::Mapping<std::string>(rdf::RDFS::comment, &comment_)) ;
      rdfmap.push_back(new rdf::Mapping<std::string>(rdf::DCT::description, &description_)) ;

      rdfmap.push_back(new rdf::Mapping<rdf::Resource>(rdf::PRV::precededBy, &precededby_)) ;
      rdfmap.push_back(new rdf::Mapping<rdf::Node>(rdf::DCT::creator, &creator_)) ;
      rdfmap.push_back(new rdf::Mapping<bsml::Datetime>(rdf::DCT::created, &created_)) ;

/*
        attributes = [ 'uri', 'precededBy', 'creator', 'created' ]
  '''List of generic attributes all resources have.'''


              'precededBy':  PropertyMap(PRV.precededBy),


              'creator':     PropertyMap(DCT.creator, to_rdf=PropertyMap.get_uri),
                  return v.uri if hasattr(v, 'uri') else Uri(v)

              set_creator(const rdf::Resource &creator)
              set_creator(const std::string &creator)


              'created':     PropertyMap(DCT.created, datatype=XSD.dateTime,
                                         to_rdf=utils.datetime_to_isoformat,
                                         from_rdf=utils.isoformat_to_datetime),
            }
*/

      }

    ~AbstractObject(void)
    /*-----------------*/
    {
      for (std::list<rdf::MapBase *>::iterator map = rdfmap.begin() ; map != rdfmap.end(); ++map)
        delete *map ;
      }

    void set_label(const std::string &label)
    /*------------------------------------*/
    {
      label_ = label ;
      }

    void set_comment(const std::string &comment)
    /*----------------------------------------*/
    {
      comment_ = comment ;
      }

    void set_description(const std::string &description)
    /*------------------------------------------------*/
    {
      description_ = description ;
      }

    const rdf::Resource *get_resource(void)
    /*-----------------------------------*/
    {
      return &resource ;
      }

    const std::string get_uri_as_string(void)
    /*-------------------------------------*/
    {
      return resource.as_string() ;
      }

    void to_rdf(const rdf::Graph &graph)
    /*--------------------------------*/
    {
      for (std::list<rdf::MapBase *>::iterator map = rdfmap.begin() ; map != rdfmap.end(); ++map)
        (*map)->to_rdf(graph, resource) ;
      }

    } ;

  } ;


#endif
