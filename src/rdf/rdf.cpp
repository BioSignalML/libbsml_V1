#include <string>
#include <raptor2/raptor2.h>

#include "rdf/rdf.h"
#include "rdf/rdfnames.h"

using namespace rdf ;


static raptor_world *rdf_world = nullptr ;

static raptor_world *world(void)
/*----------------------------*/
{
  if (rdf_world == nullptr) {    // Need to make thread safe...
    rdf_world = raptor_new_world() ;
    raptor_world_open(rdf_world) ;
    }
  return rdf_world ;
  }

void rdf::finalise(void)
/*--------------------*/
{
  if (rdf_world != nullptr) {
    raptor_free_world(rdf_world) ;
    rdf_world = nullptr ;
    }
  }


std::string Format::RDFXML = "rdfxml" ;
std::string Format::TURTLE = "turtle" ;
std::string Format::JSON   = "json" ;


/*================================================================================*/

Uri::Uri(void)
/*----------*/
: uri(nullptr) { }

Uri::Uri(const std::string &uri)
/*----------------------------*/
: uri((RDFObject)raptor_new_uri(world(), (const unsigned char *)uri.c_str()))
{
  }

Uri::~Uri(void)
/*-----------*/
{
  if (uri) raptor_free_uri((raptor_uri *)uri) ;
  }


Uri::Uri(const Uri &other)                     // Copy constructor
/*----------------------*/
: uri((RDFObject)raptor_uri_copy((raptor_uri *)(other.uri)))
{
  }

Uri::Uri(Uri &&other)                          // Move constructor
/*-----------------*/
: uri(other.uri)
{
  other.uri = nullptr ;
  }

Uri &Uri::operator=(const Uri &other)          // Copy assignment
/*---------------------------------*/
{
  if (this != &other) {
    if (uri) raptor_free_uri((raptor_uri *)uri) ;
    uri = (RDFObject)raptor_uri_copy((raptor_uri *)(other.uri)) ;
    }
  return *this ;
  }

Uri &Uri::operator=(Uri &&other)               // Move assignment
/*----------------------------*/
{
  if (this != &other) {
    if (uri) raptor_free_uri((raptor_uri *)uri) ;
    uri = (RDFObject)raptor_uri_copy((raptor_uri *)(other.uri)) ;
//    uri = other.uri ;
    }
  return *this ;
  }

bool Uri::is_empty(void) const
/*--------------------------*/
{
  return (uri == nullptr) ;
  }

std::string Uri::as_string(void) const
/*----------------------------------*/
{
  return (uri != nullptr)
        ? std::string((char *)raptor_uri_as_string((raptor_uri *)uri))
        : "NULL" ;
  }


/*================================================================================*/

Node::Node(void)
/*------------*/
: node(nullptr) { }

Node::Node(const std::string &identifier)
/*-------------------------------------*/
: node((RDFObject)raptor_new_term_from_blank(world(),
         (identifier != "") ? (const unsigned char *)identifier.c_str() : nullptr))
{
  }

Node::Node(const Uri &uri)
/*----------------------*/
: node((RDFObject)raptor_new_term_from_uri(world(), (raptor_uri *)uri.uri))
{
  }

Node::Node(const std::string &value, const std::string &language)
/*-------------------------------------------------------------*/
: node((RDFObject)raptor_new_term_from_literal(world(),
         (const unsigned char *)value.c_str(),
         nullptr,
         (const unsigned char *)language.c_str()))
{
  }

Node::Node(const std::string &value, const Resource &datatype)
/*----------------------------------------------------------*/
: node((RDFObject)raptor_new_term_from_literal(world(),
         (const unsigned char *)value.c_str(),
         (raptor_uri *)datatype.get_rdf_uri(),
         nullptr))
{
  }

Node::~Node(void)
/*-------------*/
{
  if (node) raptor_free_term((raptor_term *)node) ;
  }


Node::Node(const Node &other)                     // Copy constructor
/*-------------------------*/
: node((RDFObject)raptor_term_copy((raptor_term *)(other.node)))
{
  }

Node::Node(Node &&other)                          // Move constructor
/*--------------------*/
: node(other.node)
{
  other.node = nullptr ;
  }

Node &Node::operator=(const Node &other)          // Copy assignment
/*------------------------------------*/
{
  if (this != &other) {
    if (node) raptor_free_term((raptor_term *)node) ;
    node = (RDFObject)raptor_term_copy((raptor_term *)(other.node)) ;
    }
  return *this ;
  }

Node &Node::operator=(Node &&other)               // Move assignment
/*-------------------------------*/
{
  if (this != &other) {
    if (node) raptor_free_term((raptor_term *)node) ;
    node = (RDFObject)raptor_term_copy((raptor_term *)(other.node)) ;
//    node = other.node ;
    }
  return *this ;
  }


bool Node::operator==(const Node& other) const
/*------------------------------------------*/
{
  return (node == other.node)
   || raptor_term_equals((raptor_term *)node, (raptor_term *)other.node) ;
  }

bool Node::operator!=(const Node& other) const
/*------------------------------------------*/
{
  return !operator==(other) ;
  }

bool Node::is_empty(void) const
/*---------------------------*/
{
  if (node) {
    raptor_term *term = (raptor_term *)node ;
    return (term->type == RAPTOR_TERM_TYPE_LITERAL
         && term->value.literal.string_len == 0) ;
    }
  return true ;
  }

RDFObject Node::get_rdf_uri(void) const
/*-----------------------------------*/
{
  if (node) {
    raptor_term *term = (raptor_term *)node ;
    if (term->type == RAPTOR_TERM_TYPE_URI) return term->value.uri ;
    }
  return nullptr ;
  }

std::string Node::as_string(void) const
/*-----------------------------------*/
{
  std::string result = "NULL" ;
  if (node != nullptr) {
    char *s = (char *)raptor_term_to_string((raptor_term *)node) ;
    result = std::string(s) ;
    free(s) ;
    }
  return result ;
  }


/*================================================================================*/

Resource::Resource(void)
/*--------------------*/
: Node() { }

Resource::Resource(const std::string &uri)
/*--------------------------------------*/
{
  node = (RDFObject)raptor_new_term_from_uri_string(world(), (const unsigned char *)uri.c_str()) ;
  }

Resource::Resource(const Uri &uri)
/*------------------------------*/
: Node(uri) { }

std::string Resource::as_string(void) const
/*---------------------------------------*/
{

  if (node) {
    raptor_term *term = (raptor_term *)node ;
    if (term->type == RAPTOR_TERM_TYPE_URI)
       return std::string((char *)raptor_uri_as_string(term->value.uri)) ;
    }
  return "NULL" ;
  }


/*================================================================================*/

BlankNode::BlankNode(void)
/*----------------------*/
: Node() { }

BlankNode::BlankNode(const std::string &identifier)
/*-----------------------------------------------*/
: Node(identifier) { }


/*================================================================================*/

#if WIN32
 #define snprintf _snprintf_s      // VS 2010
#endif

Literal::Literal()
/*--------------*/
: Node("") { }

Literal::Literal(int i)
/*-------------------*/
{
  char buf[20] ;
  snprintf(buf, 20, "%d", i) ;
  node = (RDFObject)raptor_new_term_from_literal(world(),
           (const unsigned char *)buf,
           (raptor_uri *)XSD::XSD_integer.get_rdf_uri(),
           nullptr) ;
  }

Literal::Literal(double d)
/*----------------------*/
{
  char buf[20] ;
  snprintf(buf, 20, "%g", d) ;
  node = (RDFObject)raptor_new_term_from_literal(world(),
           (const unsigned char *)buf,
           (raptor_uri *)XSD::XSD_double.get_rdf_uri(),
           nullptr) ;
  }

Literal::Literal(const std::string &value, const std::string &language)
/*-------------------------------------------------------------------*/
: Node(value, language) { }

Literal::Literal(const std::string &value, const Resource &datatype)
/*----------------------------------------------------------------*/
: Node(value, datatype) { }


/*================================================================================*/

Statement::Statement(void)
/*----------------------*/
: statement(NULL)
{
  }

Statement::Statement(const Resource &subject, const Resource &predicate, const Node &object)
/*----------------------------------------------------------------------------------------*/
: statement((RDFObject)raptor_new_statement_from_nodes(world(),
              raptor_term_copy((raptor_term *)(subject.node)),
              raptor_term_copy((raptor_term *)(predicate.node)),
              raptor_term_copy((raptor_term *)(object.node)),
              NULL
           ) ) { }

Statement::Statement(const Resource &subject, const Resource &predicate, const std::string &literal)
/*------------------------------------------------------------------------------------------------*/
: statement((RDFObject)raptor_new_statement_from_nodes(world(),
              raptor_term_copy((raptor_term *)(subject.node)),
              raptor_term_copy((raptor_term *)(predicate.node)),
              raptor_term_copy((raptor_term *)(Literal(literal).node)),
              NULL
            ) ) { }

//  Statement(const BlankNode &subject, const Resource &predicate, const Node &object)
//  Statement(const Uri &subject, const Uri &predicate, const Node &object)

Statement::Statement(const Statement &other)
/*----------------------------------------*/
: statement((RDFObject)raptor_statement_copy((raptor_statement *)(other.statement))) { }

Statement::~Statement(void)
/*-----------------------*/
{
  if (statement) raptor_free_statement((raptor_statement *)statement) ;
  }


/*================================================================================*/


Graph::Graph(const std::string &uri)
/*--------------------------------*/
: statements(0)
{
  }

Graph::~Graph(void)
/*---------------*/
{
  for (std::list<Statement *>::iterator sp = statements.begin() ; sp != statements.end() ;  ++sp)
    delete *sp ;
  }

void Graph::append(const Statement &statement)
/*------------------------------------------*/
{
  statements.push_back(new Statement(statement)) ;
  }

std::string Graph::serialise(const std::string &format,
/*---------------------------------------------------*/
                             const std::string &base, std::list<Prefix> prefixes)
{
  raptor_serializer *serialiser = raptor_new_serializer(world(), format.c_str()) ;
  if (serialiser == nullptr) throw Exception("Unable to create graph serialiser") ;
  raptor_uri *base_uri ;
  if (base != "") base_uri = raptor_new_uri(world(), (const unsigned char *)base.c_str()) ;
  else            base_uri = (raptor_uri *)(uri.uri) ;
  for (std::list<Prefix>::iterator p = prefixes.begin() ;  p != prefixes.end() ;  ++p) {
//std::cout << "PFX: " << p->first << "\n" ;
    raptor_serializer_set_namespace(serialiser, (raptor_uri *)(p->second.uri), (const unsigned char *)p->first.c_str()) ;
    }

  size_t length = 0 ;
  unsigned char *serialised ;
  raptor_serializer_start_to_string(serialiser, base_uri, (void **)(&serialised), &length) ;
  for (std::list<Statement *>::iterator sp = statements.begin() ; sp != statements.end() ;  ++sp)
    raptor_serializer_serialize_statement(serialiser, (raptor_statement *)((*sp)->statement)) ;
  raptor_serializer_serialize_end(serialiser) ;

  std::string result = std::string((char *)serialised, length) ;
  if (base != "") raptor_free_uri(base_uri) ;
  raptor_free_serializer(serialiser) ;
  return result ;
  }

std::string Graph::serialise(const std::string &format)
/*---------------------------------------------------*/
{
  return Graph::serialise(format, "", std::list<Prefix>()) ;
  }

/*================================================================================*/
