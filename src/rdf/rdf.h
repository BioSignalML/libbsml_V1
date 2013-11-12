#ifndef _BSML_RDF_H
#define _BSML_RDF_H

#include <iostream>

#include <string>
#include <list>
#include <map>


namespace rdf {

  class Format
  /*--------*/
  {
   public:
    static std::string RDFXML ;
    static std::string TURTLE ;
    static std::string JSON ;
    } ;

  typedef void *RDFObject ;

  void finalise(void) ;
  /*-----------------*/

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
    virtual ~Uri(void) ;
    Uri(const Uri &other) ;                      // Copy constructor
    Uri(Uri &&other) ;                           // Move constructor
    virtual Uri &operator=(const Uri &other) ;   // Copy assignment
    virtual Uri &operator=(Uri &&other) ;        // Move assignment
    virtual bool is_empty(void) const ;
    virtual std::string as_string(void) const ;

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
    virtual ~Node(void) ;
    Node(const Node &other) ;                    // Copy constructor
    Node(Node &&other) ;                         // Move constructor
    virtual Node &operator=(const Node &other);  // Copy assignment
    virtual Node &operator=(Node &&other) ;      // Move assignment
    virtual bool operator==(const Node& other) const ;
    virtual bool operator!=(const Node& other) const ;
    virtual bool is_empty(void) const ;
    virtual std::string as_string(void) const ;

    friend class Literal ;
    friend class Resource ;
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
    Literal() ;
    Literal(int i) ;
    Literal(double d) ;
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
    virtual ~Graph(void) ;
    void append(const Statement &statement) const ;
    std::string serialise(const std::string &format, const std::string &base, std::list<Prefix> prefixes) ;
    std::string serialise(const std::string &format) ;
    } ;


  } ;

#endif
