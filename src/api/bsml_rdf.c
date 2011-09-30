/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "bsml_rdf.h"
#include "bsml_rdf_mapping.h"
#include "bsml_internal.h"


librdf_world *world = NULL ;


int bsml_uri_protocol(const char *u)
//==================================
{
  return (memcmp(u, "file:", 5) == 0 || memcmp(u, "http:", 5) == 0) ;
  }


bsml_graphstore *bsml_graphstore_create(void)
//===========================================
{
  bsml_graphstore *g = (bsml_graphstore *)calloc(sizeof(bsml_graphstore), 1) ;
  g->storage = librdf_new_storage(world, "hashes", "triples", "hash-type='memory'") ;
  g->model = librdf_new_model(world, g->storage, NULL) ;
  return g ;
  }

bsml_graphstore *bsml_graphstore_create_from_uri(const char *path, const char *format)
//====================================================================================
{
  bsml_graphstore *g = bsml_graphstore_create() ;
  if (g) {
    if (!bsml_graphstore_parse_uri(g, path, format)) return g ;
    bsml_graphstore_free(g) ;
    }
  return NULL ;
  }

bsml_graphstore *bsml_graphstore_create_from_string(const char *rdf, const char *format, const char *base)
//========================================================================================================
{
  bsml_graphstore *g = bsml_graphstore_create() ;
  if (g) {
    if (!bsml_graphstore_parse_string(g, rdf, format, base)) return g ;
    bsml_graphstore_free(g) ;
    }
  return NULL ;
  }


void bsml_graphstore_free(bsml_graphstore *g)
//===========================================
{
  if (g) {
    if (g->model) librdf_free_model(g->model) ;
    if (g->storage) librdf_free_storage(g->storage) ;
    }
  }


int bsml_graphstore_parse_uri(bsml_graphstore *g, const char *path, const char *format)
//=====================================================================================
{
  librdf_uri *uri ;
  if (bsml_uri_protocol(path)) uri = librdf_new_uri(world, (const unsigned char *)path) ;
  else {
    char fullname[PATH_MAX+7] = "file://" ;
    realpath(path, fullname+7) ;
    uri = librdf_new_uri(world, (const unsigned char *)fullname) ;
    }
  if (format == NULL) format = "turtle" ;
  librdf_parser *parser = librdf_new_parser(world, format, NULL, NULL) ;
  if (librdf_parser_parse_into_model(parser, uri, uri, g->model)) {
    fprintf(stderr, "Failed to parse: %s\n", path) ;
    return 1 ;
    }
  librdf_free_parser(parser);
  librdf_free_uri(uri) ;
  return 0 ;
  }

int bsml_graphstore_parse_string(bsml_graphstore *g, const char *rdf, const char *format, const char *base)
//=========================================================================================================
{
  int error = 0 ;
  if (format == NULL) format = "turtle" ;
  librdf_uri *baseuri = librdf_new_uri(world, (unsigned char *)base) ;
  librdf_parser *parser = librdf_new_parser(world, format, NULL, NULL) ;
  if (librdf_parser_parse_string_into_model(parser, (unsigned char *)rdf, baseuri, g->model)) {
    fprintf(stderr, "Failed to parse RDF\n") ;
    error = 1 ;
    }
  librdf_free_uri(baseuri) ;
  librdf_free_parser(parser) ;
  return error ;
  }


librdf_node *results_get_node(librdf_query_results *r, const char *k)
//===================================================================
{
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_resource(n)) return librdf_new_node_from_node(n) ;
    librdf_free_node(n) ;
    }
  return NULL ;
  }

librdf_uri *results_get_uri(librdf_query_results *r, const char *k)
//=================================================================
{
  librdf_uri *u = NULL ;
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_resource(n))
      u = librdf_new_uri_from_uri(librdf_node_get_uri(n)) ;
    librdf_free_node(n) ;
    }
  return u ;
  }

const char *results_get_string(librdf_query_results *r, const char *k)
//====================================================================
{
  const char *s = NULL ;
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_literal(n))
      s = string_copy((const char *)librdf_node_get_literal_value(n)) ;
    librdf_free_node(n) ;
    }
  return s ;
  }


int bsml_graphstore_select(bsml_graphstore *g, const char *sparql, bsml_select_results *resultfn, void *userdata)
//===============================================================================================================
{
  int error = 0 ;
  librdf_query *query = librdf_new_query(world, "sparql11-query", NULL, (unsigned char *)sparql, NULL) ;
  librdf_query_results *results = librdf_model_query_execute(g->model, query) ;
  if (results == NULL) { //...
    fprintf(stderr, "SPARQL error...\n") ;
    error = 1 ;
    }
  else {             // Check type of results...
    while (!librdf_query_results_finished(results)) {
      resultfn(results, userdata) ;
      librdf_query_results_next(results) ;
      }
    librdf_free_query_results(results) ;
    }
  librdf_free_query(query) ;
  return error ;
  }



int bsml_graphstore_construct(bsml_graphstore *g, const char *sparql, bsml_construct_results *resultfn, void *userdata)
//=====================================================================================================================
{
  int error = 0 ;
  librdf_query *query = librdf_new_query(world, "sparql11-query", NULL, (unsigned char *)sparql, NULL) ;
  librdf_query_results *results = librdf_model_query_execute(g->model, query) ;
  if (results == NULL) {
    fprintf(stderr, "SPARQL error...\n") ;
    error = 1 ;
    }
  else {
    if (results && librdf_query_results_is_graph(results)) {
      librdf_stream *stream = librdf_query_results_as_stream(results) ;
      while (!librdf_stream_end(stream)) {
        librdf_statement *stmt = librdf_stream_get_object(stream) ;
        resultfn(librdf_stream_get_object(stream), userdata) ;
        librdf_stream_next(stream) ;
        }
      librdf_free_stream(stream) ;
      }
    librdf_free_query_results(results) ;
    }
  librdf_free_query(query) ;
  return error ;
  }



void bsml_rdf_initialise(void)
//============================
{
  world = librdf_new_world() ;
  librdf_world_open(world) ;
  bsml_rdfmapping_initialise() ;
  }


void bsml_rdf_finish(void)
//========================
{
  bsml_rdfmapping_finish() ;
  if (world) librdf_free_world(world) ;
  }
