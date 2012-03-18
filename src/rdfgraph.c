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

extern bsml_world bsml ;


bsml_rdfgraph *bsml_new_rdfgraph(const char *uri)
/*===============================================*/
{
  bsml_rdfgraph *graph = ALLOCATE(bsml_rdfgraph) ;
  if (graph) {
    graph->uri = string_copy(uri) ;
    graph->storage = librdf_new_storage(bsml.world, "hashes", "triples", "hash-type='memory'") ;
    graph->model = librdf_new_model(bsml.world, graph->storage, NULL) ;
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
/*====================================================================================*/
{
  bsml_rdfgraph *graph = bsml_new_rdfgraph(uri) ;

  librdf_parser *parser = librdf_new_parser(bsml.world, "guess", NULL, NULL) ;
  librdf_uri *base = librdf_new_uri(bsml.world, (const unsigned char *)uri) ;
  librdf_uri *rdf  = librdf_new_uri(bsml.world, (const unsigned char *)rdf_uri) ;

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
