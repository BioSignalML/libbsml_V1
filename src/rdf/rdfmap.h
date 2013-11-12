#ifndef _BSML_RDFMAP_H
#define _BSML_RDFMAP_H

#include <string>

#include "rdf/rdf.h"


namespace rdf
/*=========*/
{

  class Mapping
  /*=========*/
  {
   private:
    const rdf::Resource property ;
    const rdf::Node *reference ;
    const std::string *literal ;

   public:
    Mapping(rdf::Resource &property, const rdf::Node *reference)
    /*------------------------------------------------*/
    : property(property), reference(reference), literal(nullptr) { }

    Mapping(rdf::Resource &property, const std::string *literal)
    /*------------------------------------------------*/
    : property(property), reference(nullptr), literal(literal) { }

    void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject)
    /*--------------------------------------------------------------*/
    {
      if (reference != nullptr && !reference->is_empty()) {
        graph.append(rdf::Statement(subject, property, *reference)) ;
        }
      else if (literal != nullptr && *literal != "") {
        graph.append(rdf::Statement(subject, property, *literal)) ;
        }
      }

    } ;


  class RdfMap
  /*========*/
  {
   private:
    std::list<Mapping *> maps ;

   public:
    RdfMap(void)
    /*--------*/
    : maps(std::list<Mapping *>()) { }

    virtual ~RdfMap(void)
    /*-----------------*/
    {
      for (auto m = maps.begin() ;  m != maps.end() ;  ++m) {
        delete *m ;
        }
      }

    void append(rdf::Resource &property, const rdf::Node *reference)
    /*------------------------------------------------------------*/
    {
      maps.push_back(new Mapping(property, reference)) ;
      }

    void append(rdf::Resource &property, const std::string *literal)
    /*------------------------------------------------------------*/
    {
      maps.push_back(new Mapping(property, literal)) ;
      }

    void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject)
    /*--------------------------------------------------------------*/
    {
      for (auto m = maps.begin() ;  m != maps.end() ;  ++m) {
        (*m)->to_rdf(graph, subject) ;
        }
      }

    } ;

  } ;

#endif
