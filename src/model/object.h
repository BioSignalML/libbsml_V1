#ifndef _BSML_OBJECT_H
#define _BSML_OBJECT_H

#include <string>
#include <list>

#include "rdf/rdf.h"
#include "rdf/rdfmap.h"
#include "model/datetime.h"


namespace bsml {

  class AbstractObject
  /*================*/
  {
   private:
    rdf::Resource resource ;
    rdf::Resource metatype ;
    std::string label_ ;
    std::string comment_ ;
    std::string description_ ;
    rdf::Resource precededby_ ;
    rdf::Node creator_ ;       // Literal or Resource
    Datetime created_ ;

   protected:
    std::list<rdf::MapBase *> rdfmap ;

   public:
    AbstractObject(void) ;
    AbstractObject(const rdf::Resource &type, const std::string &uri) ;
    virtual ~AbstractObject(void) ;

    void set_label(const std::string &label) ;
    void set_comment(const std::string &comment) ;
    void set_description(const std::string &description) ;
    const rdf::Resource *get_resource(void) ;
    const std::string get_uri_as_string(void) const ;
    virtual void to_rdf(const rdf::Graph &graph) ;
    } ;

  } ;


#endif
