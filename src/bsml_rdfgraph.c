/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdlib.h>
#include <redland.h>

#include "bsml-internal.h"


RDF_Nodes RDF ;

BSML_Nodes BSML ;

static librdf_world *world = NULL ;


void bsml_rdfgraph_initialise(void)
/*===============================*/
{
  if (world == NULL) {
    world = librdf_new_world() ;
    if (world == NULL) {
      fprintf(stderr, "Creating Redland RDF world failed\n") ;
      exit(1) ;
      }
    librdf_world_open(world) ;

    RDF.type = librdf_new_node_from_node(LIBRDF_MS_type(world)) ;

    BSML.Signal    = librdf_new_node_from_uri_string(world, BSML_NS("Signal")) ;
    BSML.Recording = librdf_new_node_from_uri_string(world, BSML_NS("Recording")) ;
    }
  }

void bsml_rdfgraph_finish(void)
/*===========================*/
{
  if (world) {
    librdf_free_world(world) ;
    world = NULL ;
    }
  }


bsml_rdfgraph *bsml_new_rdfgraph(const char *uri)
/*===============================================*/
{
  bsml_rdfgraph *graph = ALLOCATE(bsml_rdfgraph) ;
  if (graph) {
    graph->uri = string_copy(uri) ;
    graph->storage = librdf_new_storage(world, "hashes", "triples", "hash-type='memory'") ;
    graph->model = librdf_new_model(world, graph->storage, NULL) ;
    }
  return graph ;
  }


void bsml_free_rdfgraph(bsml_rdfgraph *graph)
/*===========================================*/
{
  if (graph) {
    if (graph->model) librdf_free_model(graph->model) ;
    if (graph->storage) librdf_free_storage(graph->storage) ;
    if (graph->uri) free((char *)graph->uri) ;
    free(graph) ;
    }
  }


bsml_rdfgraph *bsml_rdfgraph_create_and_load_rdf(const char *uri, const char *rdf_uri)
/*===================================================================================*/
{
  bsml_rdfgraph *graph = bsml_new_rdfgraph(uri) ;

  librdf_parser *parser = librdf_new_parser(world, "guess", NULL, NULL) ;
  librdf_uri *base = librdf_new_uri(world, (const unsigned char *)uri) ;
  librdf_uri *rdf  = librdf_new_uri(world, (const unsigned char *)rdf_uri) ;

  if (librdf_parser_parse_into_model(parser, rdf, base, graph->model)) {
    bsml_log_error("Failed to parse: %s\n", rdf_uri) ;
    bsml_free_rdfgraph(graph) ;
    graph = NULL ;
    }

  librdf_free_uri(rdf) ;
  librdf_free_uri(base) ;
  librdf_free_parser(parser) ;
  return graph ;
  }


unsigned char *bsml_rdfgraph_serialise(bsml_rdfgraph *graph, const char *uri, const char *mimetype)
/*===============================================================================================*/
{
  librdf_serializer *serialiser = librdf_new_serializer(world, NULL, mimetype, NULL) ;
  librdf_uri *base = (uri != NULL) ? librdf_new_uri(world, (const unsigned char *)uri) : NULL ;

  unsigned char *rdf = librdf_serializer_serialize_model_to_string(serialiser, base, graph->model) ;

  if (base) librdf_free_uri(base) ;
  librdf_free_serializer(serialiser) ;
  return rdf ;
  }


void bsml_rdfgraph_free_string(unsigned char *str)
/*==============================================*/
{
  if (str) librdf_free_memory(str) ;
  }


int bsml_rdfgraph_has_type(bsml_rdfgraph *graph, const char *uri, librdf_node *type)
/*================================================================================*/
{
  librdf_node *subj = librdf_new_node_from_uri_string(world, (const unsigned char *)uri) ;
  librdf_statement *stmt = librdf_new_statement_from_nodes(world, subj, RDF.type, type) ;

  int result = librdf_model_contains_statement(graph->model, stmt) ;

  librdf_free_statement(stmt) ;
  librdf_free_node(subj) ;
  return result ;
  }


int bsml_rdfgraph_has_signal(bsml_rdfgraph *graph, const char *uri)
/*===============================================================*/
{
  return bsml_rdfgraph_has_type(graph, uri, BSML.Signal) ;
  }

int bsml_rdfgraph_has_recording(bsml_rdfgraph *graph, const char *uri)
/*==================================================================*/
{
  return bsml_rdfgraph_has_type(graph, uri, BSML.Recording) ;
  }
