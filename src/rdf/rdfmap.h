#ifndef _BSML_RDFMAP_H
#define _BSML_RDFMAP_H


#include <string>

#include "rdf/rdf.h"


namespace rdf
/*=========*/
{

  class MapBase
  /*=========*/
  {
   protected:
    rdf::Resource property ;

   public:
    MapBase(const rdf::Resource &property)
    /*----------------------------------*/
    : property(property) { }

    virtual ~MapBase(void)
    /*-------------------*/
    { }

    virtual void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject) = 0 ;
    } ;


  template<typename T>
  class Mapping : public MapBase
  /*==========================*/
  {
   private:
    const T *reference ;

   public:
    Mapping(rdf::Resource &property, const T *reference)
    /*------------------------------------------------*/
    : MapBase(property), reference(reference) { }

    void to_rdf(const rdf::Graph &graph, const rdf::Resource &subject)
    /*--------------------------------------------------------------*/
    {
      if (*reference != T())
        graph.append(rdf::Statement(subject, property, *reference)) ;
      }

    } ;


  } ;

#endif
