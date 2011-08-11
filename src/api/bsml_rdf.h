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
  librdf_storage *storage ;
  librdf_model *model ;
  } bsml_graphstore ;


typedef void (bsml_select_results)(librdf_query_results *, void *) ;
typedef void (bsml_construct_results)(librdf_statement *, void *) ;


#ifdef __cplusplus
extern "C" {
#endif

void bsml_rdf_initialise(void) ;

int uri_protocol(const char *) ;

bsml_graphstore *bsml_graphstore_create(void) ;
bsml_graphstore *bsml_graphstore_create_from_uri(const char *, const char *) ;
bsml_graphstore *bsml_graphstore_create_from_string(const char *, const char *, const char *) ;
void bsml_graphstore_free(bsml_graphstore *) ;

int bsml_graphstore_parse_uri(bsml_graphstore *, const char *, const char *) ;
int bsml_graphstore_parse_string(bsml_graphstore *, const char *, const char *, const char *) ;

librdf_node *results_get_node(librdf_query_results *, const char *) ;
librdf_uri *results_get_uri(librdf_query_results *, const char *) ;
const char *results_get_string(librdf_query_results *, const char *) ;

int bsml_graphstore_select(bsml_graphstore *, const char *, bsml_select_results *, void *) ;
int bsml_graphstore_construct(bsml_graphstore *, const char *, bsml_construct_results *, void *) ;

#ifdef __cplusplus
} ;
#endif

#endif
