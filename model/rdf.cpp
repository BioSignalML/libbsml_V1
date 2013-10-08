#include <string>
#include <redland.h>

#include "rdf.h"

using namespace rdf ;


static librdf_world *world = NULL ;

librdf_world *rdf::get_world(void)
/*=========================*/
{
  if (world == NULL) {    // Need to make thread safe...
    world = librdf_new_world() ;
    librdf_world_open(world) ;
    }
  return world ;
  }

void rdf::end_world(void)
/*================*/
{
  if (world != NULL) {
    librdf_free_world(world) ;
    world = NULL ;
    }
  }


Uri::Uri(void)
/*-----*/
: uri(NULL) { }

Uri::Uri(const std::string &uri)
/*-----------------------*/
: uri(librdf_new_uri(
        get_world(),
        (const unsigned char *)uri.c_str()))
{
  }

Uri::Uri(const Uri &other)
/*-----------------*/
  : uri(librdf_new_uri_from_uri(other.uri)) { }

Uri::~Uri(void)
/*------*/
{
  if (uri != NULL) librdf_free_uri(uri) ;
  }

std::string Uri::as_string(void) const
/*-----------------------------*/
{
  return (uri == NULL) ? "" : std::string((char *)librdf_uri_as_string(uri)) ;
  }

librdf_uri *Uri::get_librdf_uri(void) const
/*----------------------------------*/
{
  return uri ;
  }



Node::Node(void)
/*------*/
  : node(NULL) { }

Node::Node(const std::string &identifier)
/*-------------------------------*/
  : node(librdf_new_node_from_blank_identifier(
      get_world(),
      (identifier != "") ? (const unsigned char *)identifier.c_str() : NULL)) { }

Node::Node(const Uri &uri)
/*----------------*/
  : node(librdf_new_node_from_uri(
           get_world(),
           uri.get_librdf_uri())) { }

Node::Node(const std::string &value, const std::string &language)
/*-------------------------------------------------------*/
  : node(librdf_new_node_from_literal(
           get_world(),
           (const unsigned char *)value.c_str(),
           language.c_str(),
           0)) { }

Node::Node(const std::string &value, const Resource &datatype)
/*----------------------------------------------------------*/
: node(librdf_new_node_from_typed_literal(
         get_world(),
         (const unsigned char *)value.c_str(),
         NULL,
         datatype.get_librdf_uri())) { }

Node::Node(const Node &other)
/*-------------------*/
  : node(librdf_new_node_from_node(other.node)) { }

Node::~Node(void)
/*-------*/
{
  if (node) librdf_free_node(node) ;
  }

bool Node::operator==(const Node& other) const
/*------------------------------------*/
{
  return (node == other.node) || librdf_node_equals(node, other.node) ;
  }

bool Node::operator!=(const Node& other) const
/*------------------------------------*/
{
  return !operator==(other) ;
  }

librdf_node *Node::get_librdf_node(void) const
/*------------------------------------*/
{
  return node ;
  }

librdf_uri *Node::get_librdf_uri(void) const
/*------------------------------------*/
{
  return node ? librdf_node_get_uri(node) : NULL ;
  }



Resource::Resource(void)
/*--------------------*/
: Node() { }

Resource::Resource(const std::string &uri)
/*--------------------------------------*/
 : Node(Uri(uri)) { }

Resource::Resource(const Uri &uri)
/*------------------------------*/
 : Node(uri) { }

std::string Resource::as_string(void) const
/*-----------------------------*/
{
  return std::string((char *)librdf_uri_as_string(librdf_node_get_uri(node))) ;
  }



BlankNode::BlankNode(void)
/*----------------------*/
 : Node() { }

BlankNode::BlankNode(const std::string &identifier)
/*-----------------------------------------------*/
 : Node(identifier) { }



Literal::Literal(const std::string &value, const std::string &language)
/*-------------------------------------------------------------*/
 : Node(value, language) { }

Literal::Literal(const std::string &value, const Resource &datatype)
/*-------------------------------------------------------*/
 : Node(value, datatype) { }



Statement::Statement(const Resource &subject, const Resource &predicate, const Node &object)
/*-----------------------------------------------------------------------------*/
 : statement(librdf_new_statement_from_nodes(
               get_world(),
               librdf_new_node_from_node(subject.get_librdf_node()),
               librdf_new_node_from_node(predicate.get_librdf_node()),
               librdf_new_node_from_node(object.get_librdf_node())
            ) ) { }

Statement::Statement(const Resource &subject, const Resource &predicate, const std::string &literal)
/*-------------------------------------------------------------------------------------*/
 : statement(librdf_new_statement_from_nodes(
               get_world(),
               librdf_new_node_from_node(subject.get_librdf_node()),
               librdf_new_node_from_node(predicate.get_librdf_node()),
               librdf_new_node_from_node(Literal(literal).get_librdf_node())
            ) ) { }

//  Statement(const BlankNode &subject, const Resource &predicate, const Node &object)
//  Statement(const Uri &subject, const Uri &predicate, const Node &object)

Statement::Statement(const Statement &other)
/*-----------------------------*/
 : statement(librdf_new_statement_from_statement(other.statement)) { }

Statement::~Statement(void)
/*------------*/
{
  librdf_free_statement(statement) ;
  }

librdf_statement *Statement::get_librdf_statement(void) const
/*----------------------------------------------*/
{
  return statement ;
  }


Graph::Graph(const std::string &uri)
/*-------------------------*/
: storage(librdf_new_storage(get_world(), "hashes", "triples", "hash-type='memory'")),
  model(librdf_new_model(get_world(), storage, NULL)),
  uri(Uri(uri))
{
  }

Graph::~Graph(void)
/*--------*/
{
  librdf_free_model(model) ;
  librdf_free_storage(storage) ;
  }

void Graph::append(const Statement &statement) const
/*-----------------------------------------*/
{
  librdf_model_add_statement(model, statement.get_librdf_statement()) ;
  }

std::string Graph::serialise(  // Need to specify format, base, and prefixes
  std::list<Prefix> prefixes
  )
/*-----------------------*/
{
  librdf_serializer *serialiser = librdf_new_serializer(get_world(), "turtle", NULL, NULL) ;
  librdf_uri *base ;
  base = uri.get_librdf_uri() ;
  for (std::list<Prefix>::iterator p = prefixes.begin() ;  p != prefixes.end() ;  ++p) {
//std::cout << "PFX: " << p->first << "\n" ;
    librdf_serializer_set_namespace(serialiser, p->second.get_librdf_uri(), p->first.c_str()) ;
    }
  unsigned char *serialised = librdf_serializer_serialize_model_to_string(serialiser, base, model) ;
  std::string result = std::string((char *)serialised) ;
  librdf_free_memory(serialised) ;
  librdf_free_serializer(serialiser) ;
  return result ;
  }

std::string Graph::serialise(void)
/*-----------------------*/
{
  return Graph::serialise(std::list<Prefix>()) ;
  }
