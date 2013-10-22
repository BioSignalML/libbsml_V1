#ifndef _BSML_RDF_H
#define _BSML_RDF_H

#include <iostream>

#include <string>
#include <list>
#include <map>


namespace rdf {

  typedef void *RDFObject ;

  RDFObject get_world(void) ;
  /*-----------------------*/

  void end_world(void) ;
  /*------------------*/

  class Node ;             // Declare forward
  class Graph ;
  class Resource ;
  class Statement ;

  class Uri
  /*=====*/
  {
   private:
    RDFObject uri ;

   public:
    Uri(void) ;
    Uri(const std::string &uri) ;
    Uri(const Uri &other) ;
    ~Uri(void) ;
    std::string as_string(void) const ;

    friend class Node ;
    friend class Graph ;
    } ;


  class Node
  /*======*/
  {
   protected:
    RDFObject node ;
    RDFObject get_rdf_uri(void) const ;

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

    friend class Statement ;
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
    RDFObject statement ;

   public:
    Statement(const Resource &subject, const Resource &predicate, const Node &object) ;
    Statement(const Resource &subject, const Resource &predicate, const std::string &literal) ;
//    Statement(const BlankNode &subject, const Resource &predicate, const Node &object)
//    Statement(const Uri &subject, const Uri &predicate, const Node &object)
    Statement(const Statement &other) ;
    ~Statement(void) ;

    friend class Graph ;
    } ;


  typedef std::pair<const std::string &, const Uri &> Prefix ;


  class Graph
  /*=======*/
  {
   private:
    RDFObject storage ;
    RDFObject model ;
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
