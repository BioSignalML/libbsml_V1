#ifndef _BSML_RDF_H
#define _BSML_RDF_H

#include <iostream>

#include <string>
#include <list>
#include <map>

#include <redland.h>


namespace rdf {

  librdf_world *get_world(void) ;
  /*---------------------------*/

  void end_world(void) ;
  /*------------------*/


  class Uri
  /*=====*/
  {
   private:
    librdf_uri *uri ;

   public:
    Uri(void) ;
    Uri(const std::string &uri) ;
    Uri(const Uri &other) ;
    ~Uri(void) ;
    std::string as_string(void) const ;
    librdf_uri *get_librdf_uri(void) const ;
    } ;

  class Resource ;        // Declare forward

  class Node
  /*======*/
  {
   protected:
    librdf_node *node ;

   public:
    Node(void) ;
    Node(const std::string &identifier) ;
    Node(const Uri &uri) ;
    Node(const std::string &value, const std::string &language) ;
    Node(const std::string &value, const Resource &datatype) ;
    Node(const Node &other) ;
    ~Node(void) ;
    bool operator==(const Node& other) const ;
    bool operator!=(const Node& other) const ;
    librdf_node *get_librdf_node(void) const ;
    librdf_uri *get_librdf_uri(void) const ;
    } ;


  class Resource : public Node
  /*========================*/
  {
   public:
    Resource(void) ;
    Resource(const std::string &uri) ;
    Resource(const Uri &uri) ;
    std::string as_string(void) const ;
    } ;


  class BlankNode : public Node
  /*=========================*/
  {
   public:
    BlankNode(void) ;
    BlankNode(const std::string &identifier) ;
    } ;


  class Literal : public Node
  /*=======================*/
  {
   public:
    Literal(const std::string &value, const std::string &language="") ;
    Literal(const std::string &value, const Resource &datatype) ;
    } ;


  class Statement
  /*===========*/
  {
   private:
    librdf_statement *statement ;

   public:
    Statement(const Resource &subject, const Resource &predicate, const Node &object) ;
    Statement(const Resource &subject, const Resource &predicate, const std::string &literal) ;
//    Statement(const BlankNode &subject, const Resource &predicate, const Node &object)
//    Statement(const Uri &subject, const Uri &predicate, const Node &object)
    Statement(const Statement &other) ;
    ~Statement(void) ;
    librdf_statement *get_librdf_statement(void) const ;
    } ;


  typedef std::pair<const std::string &, const Uri &> Prefix ;


  class Graph
  /*=======*/
  {
   private:
    librdf_storage *storage ;
    librdf_model *model ;
    const Uri uri ;

   public:
    Graph(const std::string &uri) ;
    ~Graph(void) ;
    void append(const Statement &statement) const ;
    std::string serialise(  // Need to specify format, base, and prefixes
      std::list<Prefix> prefixes
      ) ;
    std::string serialise(void) ;
    } ;


  } ;

#endif
