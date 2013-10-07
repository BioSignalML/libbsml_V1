#ifndef _BSML_RDF_H
#define _BSML_RDF_H

#include <string>

#include <redland.h>


namespace rdf {

  static librdf_world *world = NULL ;

  class World
  /*========*/
  {
   public:
    World(void)
    /*-------*/
    {
      if (world == NULL) {
        world = librdf_new_world() ;
        librdf_world_open(world) ;
        }
      }
    ~World(void)
    /*--------*/
    {
      if (world != NULL) {
        librdf_free_world(world) ;
        world = NULL ;
        }
      }
    } ;

  static World RDF_World ;


  class Uri
  /*=====*/
  {
   private:
    librdf_uri *uri ;

   public:
    Uri(void)
    /*-----*/
    : uri(NULL) { }

    Uri(const std::string &uri)
    /*-----------------------*/
    : uri(librdf_new_uri(
            world,
            (const unsigned char *)uri.c_str()))
    {
      }

    Uri(const Uri &other)
    /*-----------------*/
      : uri(librdf_new_uri_from_uri(other.uri)) { }

    ~Uri(void)
    /*------*/
    {
      if (uri != NULL) librdf_free_uri(uri) ;
      }

    std::string as_string(void) const
    /*-----------------------------*/
    {
      return (uri == NULL) ? "" : std::string((char *)librdf_uri_as_string(uri)) ;
      }

    librdf_uri *get_librdf_uri(void) const
    /*----------------------------------*/
    {
      return uri ;
      }

    } ;

  class Resource ;        // Declare forward

  class Node
  /*======*/
  {
   protected:
    librdf_node *node ;

   public:

    Node(void)
    /*------*/
      : node(librdf_new_node(world)) { }

    Node(const std::string &identifier)
    /*-------------------------------*/
      : node(librdf_new_node_from_blank_identifier(
          world,
          (const unsigned char *)identifier.c_str())) { }

    Node(const Uri &uri)
    /*----------------*/
      : node(librdf_new_node_from_uri(
               world,
               uri.get_librdf_uri())) { }

    Node(const std::string &value, const std::string &language)
    /*-------------------------------------------------------*/
      : node(librdf_new_node_from_literal(
               world,
               (const unsigned char *)value.c_str(),
               language.c_str(),
               0)) { }

    Node(const std::string &value, const Resource &datatype) ;
    /*------------------------------------------------------*/

    Node(const Node &other)
    /*-------------------*/
      : node(librdf_new_node_from_node(other.node)) { }

    ~Node(void)
    /*-------*/
    {
      librdf_free_node(node) ;
      }

    bool operator==(const Node& other) const
    /*------------------------------------*/
    {
      return librdf_node_equals(node, other.node) ;
      }

    bool operator!=(const Node& other) const
    /*------------------------------------*/
    {
      return !operator==(other) ;
      }

    librdf_node *get_librdf_node(void) const
    /*------------------------------------*/
    {
      return node ;
      }

    librdf_uri *get_librdf_uri(void) const
    /*------------------------------------*/
    {
      return librdf_node_get_uri(node) ;
      }

    } ;


  class Resource : public Node
  /*========================*/
  {
   public:
    Resource(void)
    /*----------*/
    : Node() { }

    Resource(const std::string &uri)
    /*----------------------------*/
     : Node(Uri(uri)) { }

    Resource(const Uri &uri)
    /*--------------------*/
     : Node(uri) { }

    std::string as_string(void) const
    /*-----------------------------*/
    {
      return std::string((char *)librdf_uri_as_string(librdf_node_get_uri(node))) ;
      }

    } ;


  class BlankNode : public Node
  /*=========================*/
  {
   public:

    BlankNode(void)
    /*-----------*/
     : Node() { }

    BlankNode(const std::string &identifier)
    /*------------------------------------*/
     : Node(identifier) { }

    } ;


  class Literal : public Node
  /*=======================*/
  {
   public:

    Literal(const std::string &value, const std::string &language="")
    /*-------------------------------------------------------------*/
     : Node(value, language) { }

    Literal(const std::string &value, const Resource &datatype)
    /*-------------------------------------------------------*/
     : Node(value, datatype) { }

    } ;


  class Statement
  /*===========*/
  {
   private:
    librdf_statement *statement ;

   public:
    Statement(const Resource &subject, const Resource &predicate, const Node &object)
    /*-----------------------------------------------------------------------------*/
     : statement(librdf_new_statement_from_nodes(
                   world,
                   librdf_new_node_from_node(subject.get_librdf_node()),
                   librdf_new_node_from_node(predicate.get_librdf_node()),
                   librdf_new_node_from_node(object.get_librdf_node())
                ) ) { }

    Statement(const Resource &subject, const Resource &predicate, const std::string &literal)
    /*-------------------------------------------------------------------------------------*/
     : statement(librdf_new_statement_from_nodes(
                   world,
                   librdf_new_node_from_node(subject.get_librdf_node()),
                   librdf_new_node_from_node(predicate.get_librdf_node()),
                   librdf_new_node_from_node(Literal(literal).get_librdf_node())
                ) ) { }

//    Statement(const BlankNode &subject, const Resource &predicate, const Node &object)
//    Statement(const Uri &subject, const Uri &predicate, const Node &object)


    Statement(const Statement &other)
    /*-----------------------------*/
     : statement(librdf_new_statement_from_statement(other.statement)) { }

    ~Statement(void)
    /*------------*/
    {
      librdf_free_statement(statement) ;
      }

    librdf_statement *get_librdf_statement(void) const
    /*----------------------------------------------*/
    {
      return statement ;
      }

    } ;


  class Graph
  /*=======*/
  {
   private:
    librdf_storage *storage ;
    librdf_model *model ;
    const Uri uri ;

   public:
    Graph(const std::string &uri)
    /*-------------------------*/
    : storage(librdf_new_storage(world, "hashes", "triples", "hash-type='memory'")),
      model(librdf_new_model(world, storage, NULL)),
      uri(Uri(uri))
    {
      }

    ~Graph(void)
    /*--------*/
    {
      librdf_free_model(model) ;
      librdf_free_storage(storage) ;
      }

    void append(const Statement &statement) const
    /*-----------------------------------------*/
    {
      librdf_model_add_statement(model, statement.get_librdf_statement()) ;
      }

    std::string serialise(void)  // Need to specify format, base, and prefixes
    /*-----------------------*/
    {
      librdf_serializer *serialiser = librdf_new_serializer(world, "turtle", NULL, NULL) ;
      librdf_uri *base ;
      base = uri.get_librdf_uri() ;

//    for prefix, uri in prefixes.iteritems():
//      serialiser.set_namespace(prefix, Uri(uri))
//
//      librdf_serializer_set_namespace     (serialiser,
//                                                         librdf_uri *uri,
//                                                         const char *prefix)
      unsigned char *serialised = librdf_serializer_serialize_model_to_string(serialiser, base, model) ;
      std::string result = std::string((char *)serialised) ;
      librdf_free_memory(serialised) ;
      librdf_free_serializer(serialiser) ;
      return result ;
      }

    } ;


  } ;

#endif
