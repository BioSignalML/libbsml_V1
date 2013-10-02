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

   protected:
    std::list<rdf::Mapping *> rdfmap ;     

   public:
    AbstractObject(const rdf::Resource &type, const std::string &uri)
    /*=============================================================*/
    : resource(rdf::Resource(uri)), metatype(type), rdfmap(), label_("")
    {
      rdfmap.push_back(new rdf::ResourceMap(rdf::RDF::type, metatype)) ;
      rdfmap.push_back(new rdf::StringMap(rdf::RDFS::label, label_)) ;
      }

    ~AbstractObject(void)
    /*=================*/
    {
      for (std::list<rdf::Mapping *>::iterator map = rdfmap.begin() ; map != rdfmap.end(); ++map)
        delete *map ;
      }

    void set_label(const std::string label)
    /*-----------------------------------*/
    {
      label_ = label ;
      }

    void to_rdf(const rdf::Graph &graph)
    /*================================*/
    {
      for (std::list<rdf::Mapping *>::iterator map = rdfmap.begin() ; map != rdfmap.end(); ++map)
        (*map)->to_rdf(graph, resource) ;
      }


    } ;

  } ;


#endif
