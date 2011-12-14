/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_RDF_H
#define _BSML_RDF_H

#include <redland.h>

#include "bsml_names.h"
#include "bsml_rdfnames.h"

typedef struct {
  const char *location ;
  librdf_storage *storage ;
  librdf_model *model ;
  } bsml_graph ;


typedef void (bsml_select_results)(librdf_query_results *, void *) ;
typedef void (bsml_construct_results)(librdf_statement *, void *) ;


#ifdef __cplusplus
extern "C" {
#endif

void bsml_rdf_initialise(const char *repo) ;
void bsml_rdf_finish(void) ;

int uri_protocol(const char *) ;

bsml_graph *bsml_graph_create(void) ;
bsml_graph *bsml_graph_create_from_uri(const char *, const char *) ;
bsml_graph *bsml_graph_create_from_string(const char *, const char *, const char *) ;
void bsml_graph_free(bsml_graph *) ;

int bsml_graph_parse_uri(bsml_graph *, const char *, const char *) ;
int bsml_graph_parse_string(bsml_graph *, const char *, const char *, const char *) ;

librdf_node *results_get_node(librdf_query_results *, const char *) ;
librdf_uri *results_get_uri(librdf_query_results *, const char *) ;
const char *results_get_string(librdf_query_results *, const char *) ;

int bsml_graph_select(bsml_graph *, const char *, bsml_select_results *, void *) ;
int bsml_graph_construct(bsml_graph *, const char *, bsml_construct_results *, void *) ;

#ifdef __cplusplus
} ;
#endif

#endif
