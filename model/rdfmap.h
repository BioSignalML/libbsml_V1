#ifndef _BSML_RDFMAP_H
#define _BSML_RDFMAP_H


#include <string>

#include "rdf.h"


namespace rdf
/*=========*/
{

  class Mapping
  /*=========*/
  {
   protected:
    rdf::Resource property ;

   public:
    Mapping(const rdf::Resource &property)
    /*----------------------------------*/
    : property(property) { }

    virtual void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject) { } ;
    } ;


  class StringMap : public Mapping
  /*============================*/
  {
   private:
    const std::string &str_ref ;

   public:
    StringMap(rdf::Resource &property, const std::string &string_ref)
    /*---------------------------------------------------------*/
    : Mapping(property), str_ref(string_ref) { }

    void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject)
    /*--------------------------------------------------------------*/
    {
      if (str_ref != "")
        graph.append(rdf::Statement(subject, property, rdf::Literal(str_ref))) ;
      }

    } ;


  class ResourceMap : public Mapping
  /*==============================*/
  {
   private:
    const rdf::Resource &resource ;

   public:
    ResourceMap(const rdf::Resource &property, const rdf::Resource &resource)
    /*---------------------------------------------------------------------*/
    : Mapping(property), resource(resource) { }

    void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject)
    /*--------------------------------------------------------------*/
    {
      graph.append(rdf::Statement(subject, property, resource)) ;
      }

    } ;

  } ;

#endif
