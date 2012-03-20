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

#include "bsml-internal.h"


bsml_recording *bsml_new_recording(const char *uri)
/*===============================================*/
{
  bsml_recording *rec = ALLOCATE(bsml_recording) ;
  if (rec) rec->uri = string_copy(uri) ;
  return rec ;
  }


void bsml_free_recording(bsml_recording *rec)
/*=========================================*/
{
  if (rec) {
    if (rec->graph) bsml_free_rdfgraph(rec->graph) ;
    if (rec->uri) free((char *)rec->uri) ;
    free(rec) ;
    }
  }


bsml_recording *bsml_recording_open(const char *uri)
/*================================================*/
{
  bsml_recording *rec = bsml_new_recordng(uri) ;
  if (rec) {
    bsml_rdfgraph *graph = bsml_rdfgraph_create_and_load_rdf(uri, rdf_uri) ;
    if (bsml_rdfgraph_contains(graph, repo->uri, RDF.type, BSML.Recording))
    // check it has <uri> a Recording
    rec->graph = graph ;
    }
  return rec ;
  }


bsml_recording *bsml_recording_from_repository(bsml_repository *repo, const char *uri)
/*==================================================================================*/
{
  bsml_recording *rec = bsml_new_recordng(uri) ;
  if (rec) {
    bsml_rdfgraph *graph = bsml_repository_get_metadata(repo, uri) ;
    // check it has <uri> a Recording
    rec->graph = graph ;
    }
  return rec ;
  }
