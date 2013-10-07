#ifndef _BSML_OBJECT_H
#define _BSML_OBJECT_H

#include <string>
#include <list>

#include "bsml.h"

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

   protected:
    std::list<rdf::MapBase *> rdfmap ;

   public:
    AbstractObject(const rdf::Resource &type, const std::string &uri)
    /*-------------------------------------------------------------*/
    : resource(rdf::Resource(uri)), metatype(type), rdfmap()
    {
      rdfmap.push_back(new rdf::ResourceMap(rdf::RDF::type, metatype)) ;
      rdfmap.push_back(new rdf::StringMap(rdf::RDFS::label, label_)) ;

/*
        attributes = [ 'uri', 'label', 'comment', 'description', 'precededBy', 'creator', 'created' ]
  '''List of generic attributes all resources have.'''

  mapping = { 'label':       PropertyMap(RDFS.label),
              'comment':     PropertyMap(RDFS.comment),
              'description': PropertyMap(DCT.description),
              'precededBy':  PropertyMap(PRV.precededBy),
              'creator':     PropertyMap(DCT.creator, to_rdf=PropertyMap.get_uri),
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

    void set_label(const std::string label)
    /*------------------------------------*/
    {
      label_ = label ;
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
