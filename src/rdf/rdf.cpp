#include <string>
#include <redland.h>

#include "rdf/rdf.h"
#include "rdf/rdfnames.h"

using namespace rdf ;


static librdf_world *rdf_world = nullptr ;

static librdf_world *world(void)
/*----------------------------*/
{
  if (rdf_world == nullptr) {    // Need to make thread safe...
    rdf_world = librdf_new_world() ;
    librdf_world_open(rdf_world) ;
    }
  return rdf_world ;
  }

void rdf::finalise(void)
/*--------------------*/
{
  if (rdf_world != nullptr) {
    librdf_free_world(rdf_world) ;
    rdf_world = nullptr ;
    }
  }



/*================================================================================*/

Uri::Uri(void)
/*----------*/
: uri(nullptr) { }

Uri::Uri(const std::string &uri)
/*----------------------------*/
: uri((RDFObject)librdf_new_uri(world(), (const unsigned char *)uri.c_str()))
{
  }

Uri::~Uri(void)
/*-----------*/
{
  if (uri) librdf_free_uri((librdf_uri *)uri) ;
  }


Uri::Uri(const Uri &other)                     // Copy constructor
/*----------------------*/
: uri((RDFObject)librdf_new_uri_from_uri((librdf_uri *)(other.uri)))
{
  }
std::string Uri::as_string(void) const
/*----------------------------------*/
{
  return (uri != nullptr)
        ? std::string((char *)librdf_uri_as_string((librdf_uri *)uri))
        : "NULL" ;
  }


/*================================================================================*/

Node::Node(void)
/*------------*/
: node(nullptr) { }

Node::Node(const std::string &identifier)
/*-------------------------------------*/
: node((RDFObject)librdf_new_node_from_blank_identifier(world(),
         (identifier != "") ? (const unsigned char *)identifier.c_str() : nullptr))
{
  }

Node::Node(const Uri &uri)
/*----------------------*/
: node((RDFObject)librdf_new_node_from_uri(world(), (librdf_uri *)uri.uri))
{
  }

Node::Node(const std::string &value, const std::string &language)
/*-------------------------------------------------------------*/
: node((RDFObject)librdf_new_node_from_literal(world(),
         (const unsigned char *)value.c_str(),
         language.c_str(),
         0))
{
  }

Node::Node(const std::string &value, const Resource &datatype)
/*----------------------------------------------------------*/
: node((RDFObject)librdf_new_node_from_typed_literal(world(),
         (const unsigned char *)value.c_str(),
         nullptr,
         (librdf_uri *)datatype.get_rdf_uri()))
{
  }

Node::~Node(void)
/*-------------*/
{
  if (node) librdf_free_node((librdf_node *)node) ;
  }


Node::Node(const Node &other)                     // Copy constructor
/*-------------------------*/
: node((RDFObject)librdf_new_node_from_node((librdf_node *)(other.node)))
{
  }
bool Node::operator==(const Node& other) const
/*------------------------------------*/
{
  return (node == other.node)
   || librdf_node_equals((librdf_node *)node, (librdf_node *)other.node) ;
  }

bool Node::operator!=(const Node& other) const
/*------------------------------------------*/
{
  return !operator==(other) ;
  }

RDFObject Node::get_rdf_uri(void) const
/*-----------------------------------*/
{
  return node ? (RDFObject)librdf_node_get_uri((librdf_node *)node) : nullptr ;
  }


/*================================================================================*/

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
/*---------------------------------------*/
{
  return (node != nullptr)
        ? std::string((char *)librdf_uri_as_string(librdf_node_get_uri((librdf_node *)node)))
        : "NULL" ;
  }


/*================================================================================*/

BlankNode::BlankNode(void)
/*----------------------*/
: Node() { }

BlankNode::BlankNode(const std::string &identifier)
/*-----------------------------------------------*/
: Node(identifier) { }


/*================================================================================*/

Literal::Literal()
/*--------------*/
: Node(NULL) { }

Literal::Literal(int i)
/*-------------------*/
: Node(std::to_string(i), XSD::XSD_integer) { }

Literal::Literal(double d)
/*----------------------*/
: Node(std::to_string(d), XSD::XSD_double) { }

Literal::Literal(const std::string &value, const std::string &language)
/*-------------------------------------------------------------------*/
: Node(value, language) { }

Literal::Literal(const std::string &value, const Resource &datatype)
/*----------------------------------------------------------------*/
: Node(value, datatype) { }


/*================================================================================*/

Statement::Statement(const Resource &subject, const Resource &predicate, const Node &object)
/*----------------------------------------------------------------------------------------*/
: statement((RDFObject)librdf_new_statement_from_nodes(world(),
              librdf_new_node_from_node((librdf_node *)(subject.node)),
              librdf_new_node_from_node((librdf_node *)(predicate.node)),
              librdf_new_node_from_node((librdf_node *)(object.node))
           ) ) { }

Statement::Statement(const Resource &subject, const Resource &predicate, const std::string &literal)
/*------------------------------------------------------------------------------------------------*/
: statement((RDFObject)librdf_new_statement_from_nodes(world(),
              librdf_new_node_from_node((librdf_node *)(subject.node)),
              librdf_new_node_from_node((librdf_node *)(predicate.node)),
              librdf_new_node_from_node((librdf_node *)(Literal(literal).node))
            ) ) { }

//  Statement(const BlankNode &subject, const Resource &predicate, const Node &object)
//  Statement(const Uri &subject, const Uri &predicate, const Node &object)

Statement::Statement(const Statement &other)
/*----------------------------------------*/
: statement((RDFObject)librdf_new_statement_from_statement((librdf_statement *)(other.statement))) { }

Statement::~Statement(void)
/*-----------------------*/
{
  librdf_free_statement((librdf_statement *)statement) ;
  }


/*================================================================================*/

Graph::Graph(const std::string &uri)
/*--------------------------------*/
: storage((RDFObject)librdf_new_storage((librdf_world *)world(), "hashes", "triples", "hash-type='memory'")),
  model((RDFObject)librdf_new_model((librdf_world *)world(), (librdf_storage *)storage, nullptr)),
  uri(Uri(uri))
{
  }

Graph::~Graph(void)
/*---------------*/
{
  librdf_free_model((librdf_model *)model) ;
  librdf_free_storage((librdf_storage *)storage) ;
  }

void Graph::append(const Statement &statement) const
/*------------------------------------------------*/
{
  librdf_model_add_statement((librdf_model *)model, (librdf_statement *)(statement.statement)) ;
  }

std::string Graph::serialise(  // Need to specify format, base, and prefixes
  std::list<Prefix> prefixes
  )
/*-----------------------*/
{
  librdf_serializer *serialiser = librdf_new_serializer((librdf_world *)get_world(), "turtle", NULL, NULL) ;
  librdf_uri *base ;
  base = (librdf_uri *)(uri.uri) ;
  for (std::list<Prefix>::iterator p = prefixes.begin() ;  p != prefixes.end() ;  ++p) {
//std::cout << "PFX: " << p->first << "\n" ;
    librdf_serializer_set_namespace(serialiser, (librdf_uri *)(p->second.uri), p->first.c_str()) ;
    }
  unsigned char *serialised
    = librdf_serializer_serialize_model_to_string(serialiser, base, (librdf_model *)model) ;
  std::string result = std::string((char *)serialised) ;
  librdf_free_memory(serialised) ;
  librdf_free_serializer(serialiser) ;
  return result ;
  }

std::string Graph::serialise(void)
/*------------------------------*/
{
  return Graph::serialise(std::list<Prefix>()) ;
  }


/*================================================================================*/
