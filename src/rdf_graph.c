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

#include "biosignalml.h"
#include "biosignalml-internal.h"

librdf_world *world = NULL ;


bsml_rdf_graph *bsml_new_rdf_graph(const char *uri)
/*===============================================*/
{
  bsml_rdf_graph *graph = ALLOCATE(bsml_rdf_graph) ;
  if (graph) {
    graph->uri = string_copy(uri) ;
    graph->storage = librdf_new_storage(world, "hashes", "triples", "hash-type='memory'") ;
    graph->model = librdf_new_model(world, graph->storage, NULL) ;
    }
  return graph ;
  }


void bsml_free_rdf_graph(bsml_rdf_graph *graph)
/*===========================================*/
{
  if (graph) {
    if (graph->model) librdf_free_model(graph->model) ;
    if (graph->storage) librdf_free_storage(graph->storage) ;
    if (graph->uri) free((char *)graph->uri) ;
    free(graph) ;
    }
  }


bsml_rdf_graph *bsml_rdf_graph_create_and_load_rdf(const char *uri, const char *rdf_uri)
/*====================================================================================*/
{
  bsml_rdf_graph *graph = bsml_new_rdf_graph(uri) ;

  librdf_parser *parser = librdf_new_parser(world, "guess", NULL, NULL) ;
  librdf_uri *base = librdf_new_uri(world, (const unsigned char *)uri) ;
  librdf_uri *rdf  = librdf_new_uri(world, (const unsigned char *)rdf_uri) ;

  if (librdf_parser_parse_into_model(parser, rdf, base, graph->model)) {
    bsml_log_error("Failed to parse: %s\n", rdf_uri) ;
    bsml_free_rdf_graph(graph) ;
    graph = NULL ;
    }

  librdf_free_uri(rdf) ;
  librdf_free_uri(base) ;
  librdf_free_parser(parser) ;
  return graph ;
  }


void bsml_rdf_initialise(void)
/*==========================*/
{
  world = librdf_new_world() ;
  librdf_world_open(world) ;
  //bsml_rdfmapping_initialise() ;
  }


void bsml_rdf_finish(void)
//========================
{
  //bsml_rdfmapping_finish() ;
  if (world) librdf_free_world(world) ;
  }

