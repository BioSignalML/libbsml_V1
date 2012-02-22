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
#include <string.h>

#include "biosignalml.h"
#include "biosignalml-internal.h"


bsml_repository *bsml_new_repository(const char *uri)
/*=================================================*/
{
  bsml_repository *repo = ALLOCATE(bsml_repository) ;
  if (repo) {
    repo->uri = string_copy(uri) ;
    const char *last = uri + strlen(uri) - 1 ;
    int noslash = (*last == '/' || *last == '#') ;
    repo->metadata_uri = string_cat(uri, noslash ? "metadata/" : "/metadata/") ;
    repo->stream_uri   = string_cat(uri, noslash ? "stream/"   : "/stream/") ;
    }
  return repo ;
  }


void bsml_free_repository(bsml_repository *repo)
/*============================================*/
{
  if (repo) {
    if (repo->graph) bsml_free_rdf_graph(repo->graph) ;
    if (repo->uri) free((char *)repo->uri) ;
    if (repo->stream_uri) free((char *)repo->stream_uri) ;
    if (repo->metadata_uri) free((char *)repo->metadata_uri) ;
    free(repo) ;
    }
  }


bsml_repository *bsml_repository_connect(const char *uri)
/*=====================================================*/
{
  bsml_repository *repo = bsml_new_repository(uri) ;
  if (repo) repo->graph = bsml_rdf_graph_create_and_load_rdf(uri, repo->metadata_uri) ;
  return repo ;
  }


bsml_rdf_graph *bsml_repository_get_metadata(bsml_repository *repo, const char *uri)
/*================================================================================*/
{
  if (repo) {
    const char *metadata = string_cat(repo->metadata_uri, uri) ;
    if (metadata) {
      bsml_rdf_graph *graph = bsml_rdf_graph_create_and_load_rdf(uri, metadata) ;
      free((char *)metadata) ;
      return graph ;
      }
    }
  return NULL ;
  }
