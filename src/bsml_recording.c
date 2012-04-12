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

#include "bsml_recording.h"
#include "bsml_internal.h"
#include "utility/bsml_string.h"


bsml_recording *bsml_recording_alloc(const char *uri)
/*=================================================*/
{
  bsml_recording *rec = ALLOCATE(bsml_recording) ;
  if (rec) rec->uri = bsml_string_copy(uri) ;
  return rec ;
  }


void bsml_recording_close(bsml_recording *rec)
/*==========================================*/
{
  if (rec) {
    if (rec->graph) bsml_free_rdfgraph(rec->graph) ;
    bsml_string_free(rec->uri) ;
    free(rec) ;
    }
  }


bsml_recording *bsml_recording_open(const char *uri)
/*================================================*/
{
  bsml_recording *rec = bsml_recording_alloc(uri) ;
  if (rec) {
    bsml_rdfgraph *graph = bsml_rdfgraph_create_and_load_rdf(uri, rdf_uri) ;
    if (bsml_rdfgraph_contains(graph, repo->uri, RDF.type, BSML.Recording))
    // check it has <uri> a Recording
    rec->graph = graph ;
    }
  return rec ;
  }


bsml_recording *bsml_recording_create_from_graph(const char *uri, bsml_rdfgraph *graph, bsml_repository *repo)
/*==================================================================================*/
{
  bsml_recording *rec = bsml_recording_alloc(uri) ;
  if (rec) {
    bsml_rdfgraph *graph = bsml_repository_get_metadata(repo, uri) ;
    // check it has <uri> a Recording
    rec->graph = graph ;
    }
  return rec ;
  }
