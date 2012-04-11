#ifndef _BSML_RDFGRAPH_H
#define _BSML_RDFGRAPH_H

#include <redland.h>

#ifdef __cplusplus
extern "C" {
#endif

// Mime types
#define MIMETYPE_RDFXML "application/rdf+xml"
#define MIMETYPE_TURTLE "text/turtle"
#define MIMETYPE_JSON   "application/json"


#define BSML_NAMESPACE "http://www.biosignalml.org/ontologies/2011/04/biosignalml#"
#define BSML_NS(n)     (BSML_NAMESPACE n)

typedef struct {
  librdf_node *Signal ;
  librdf_node *Recording ;
  } BSML_Nodes ;

typedef struct {
  librdf_node *type ;
  } RDF_Nodes ;

/*!
 * \defgroup rdf_graphs RDF Graphs
 * @{
 */

/*! A RDF graph. */
struct bsml_RDFgraph {
  const char *uri ;           /*!< The graph's URI */
  librdf_storage *storage ;   /*!< Where the graph is stored */
  librdf_model *model ;       /*!< The statements in the graph */
  } ;


void bsml_rdfgraph_initialise(void) ;

void bsml_rdfgraph_finish(void) ;


/*!
 * Create a rdf_graph object.
 *
 * @param uri The URI of the RDF graph
 * @return A ::bsml_rdf_graph, or NULL if errors
 */ 
bsml_rdfgraph *bsml_new_rdfgraph(const char *uri) ;

/*!
 * Free a rdf_graph object.
 *
 * @param graph The ::bsml_rdf_graph to free
 */ 
void bsml_free_rdfgraph(bsml_rdfgraph *graph) ;

/*!
 * Create a rdf_graph from RDF at an URI.
 *
 * @param uri The URI of the RDF graph
 * @param rdf_uri The URI of the graph's RDF
 * @return A ::bsml_rdf_graph, or NULL if errors
 */ 
bsml_rdfgraph *bsml_rdfgraph_create_and_load_rdf(const char *uri, const char *rdf_uri) ;


unsigned char *bsml_rdfgraph_serialise(bsml_rdfgraph *graph, const char *uri, const char *mimetype) ;

void bsml_rdfgraph_free_string(unsigned char *str) ;

/*!@}*/

#ifdef __cplusplus
  } ;
#endif

#endif
