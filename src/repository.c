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

#include <curl/curl.h>

#include "bsml-internal.h"


void bsml_repository_initialise(void)
/*=================================*/
{
  curl_global_init(CURL_GLOBAL_ALL) ;
  }


void bsml_repository_finish(void)
/*=============================*/
{
  curl_global_cleanup() ;
  }


bsml_repository *bsml_repository_connect(const char *uri)
/*=====================================================*/
{
  bsml_repository *repo = ALLOCATE(bsml_repository) ;
  if (repo) {
    repo->uri = string_copy(uri) ;
    repo->metadata_end = string_cat(uri, REPOSITORY_METADATA) ;
    repo->graph = bsml_repository_get_metadata(repo, "") ;
    }
  return repo ;
  }


void bsml_repository_close(bsml_repository *repo)
/*=============================================*/
{
  if (repo) {
    if (repo->graph) bsml_free_rdfgraph(repo->graph) ;
    if (repo->uri) free((char *)repo->uri) ;
    if (repo->metadata_end) free((char *)repo->metadata_end) ;
    free(repo) ;
    }
  }


bsml_rdfgraph *bsml_repository_get_metadata(bsml_repository *repo, const char *uri)
/*================================================================================*/
{
  if (repo) {
    const char *md_uri = string_cat(repo->metadata_end, uri) ;
    if (md_uri) {
      bsml_rdfgraph *graph = bsml_rdfgraph_create_and_load_rdf(uri, md_uri) ;
      free((char *)md_uri) ;
      return graph ;
      }
    }
  return NULL ;
  }


static int send_metadata(bsml_repository *repo, const char *uri, bsml_rdfgraph *graph, HTTP_METHOD method)
/*======================================================================================================*/
{
  CURL *curl = curl_easy_init() ;
  long status = 0 ;
  if (curl) {
    const char *md_uri = string_cat(repo->metadata_end, uri) ;
    curl_easy_setopt(curl, CURLOPT_URL, md_uri) ;

    struct curl_slist *slist = NULL ;
    const char *ctype = string_cat("Content-Type: ", MIMETYPE_RDFXML) ;
    slist = curl_slist_append(slist, ctype) ;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist) ;

    unsigned char *body = bsml_rdfgraph_serialise(graph, uri, MIMETYPE_RDFXML) ;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)body) ;
    if (method == HTTP_PUT) curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L) ;

    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L) ;
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L) ;
    int res = curl_easy_perform(curl) ;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) ;

    curl_slist_free_all(slist) ;
    bsml_rdfgraph_free_string(body) ;
    free((char *)ctype) ;
    free((char *)md_uri) ;
    curl_easy_cleanup(curl) ;
    }
  return (status != 200 || status != 201) ;
  }


static int put_metadata(bsml_repository *repo, const char *uri, bsml_rdfgraph *graph)
/*=================================================================================*/
{
  return send_metadata(repo, uri, graph, HTTP_PUT) ;
  }


static int post_metadata(bsml_repository *repo, const char *uri, bsml_rdfgraph *graph)
/*==================================================================================*/
{
  return send_metadata(repo, uri, graph, HTTP_POST) ;
  }



bsml_recording *bsml_repository_get_recording(bsml_repository *repo, const char *uri)
/*=================================================================================*/
{
  bsml_rdfgraph *graph = bsml_repository_get_metadata(repo, uri) ;
  bsml_recording *rec = NULL ;

  if (graph && bsml_rdfgraph_has_recording(graph, uri)) { // (uri, RDF.type, BSML.Recording) in graph
    rec = bsml_recording_create_from_graph(uri, graph, repo) ;
    }

  return rec ;
  }

bsml_recording *bsml_repository_get_recording_with_signals(bsml_repository *repo, const char *uri)
/*==============================================================================================*/
{
  bsml_recording *rec = bsml_repository_get_recording(repo, uri) ;
  if (rec) {

   /*
      for sig in rec.graph.get_subjects(BSML.recording, rdf.Uri(uri)):
        rec.set_signal(Signal.create_from_graph(sig.uri, self.get_metadata(sig.uri), repository=self))
    */

    }
  return rec ;
  }


/*

  def new_recording(self, uri, **kwds):
  #------------------------------------
    try:
      rec = Recording(uri, repository=self, **kwds)
      # Will have default metadata with attributes set from any metadata keyword dict
      self.put_metadata(rec.uri, rec.metadata_as_graph())
      # Format = HDF5 (BSML ??)
      # then when server processes PUT for a new BSML recording it will create an empty HDF5 container
      return rec
    except Exception, msg:
      raise IOError("Cannot create Recording '%s' in repository" % uri)


  def store_recording(self, uri, **kwds):       ## or save_recording ??
  #------------------------------------
    rec = Recording(uri, repository=self, **kwds)
    # Will have default metadata with attributes set from any metadata keyword dict
    self.put_metadata(rec.uri, rec.save_to_graph())


  def get_signal(self, uri, **kwds):
  #---------------------------------
    graph = self.get_metadata(uri)
    if (graph && bsml_rdfgraph_has_signal(graph, uri)) { // (uri, RDF.type, BSML.Signal) in graph
      sig = Signal.create_from_graph(uri, self.get_metadata(uri), repository=self)
      sig.recording = self.get_recording(sig.recording)
      return sig


*/


static void *get_data(bsml_repository *repo, const char *uri, double start, double duration,
/*========================================================================================*/
                                                             long offset, long count)
{
  stream_data *strm = stream_data_request(host, port,
    REPOSITORY_STREAM, uri, start, duration) ;

  stream_block *sb ;

  while ((sb = stream_data_read(strm)) != NULL) {

    if (sb->type == 'D') {

      // get dtype from sb->header
      // cJSON *header ;

      dtype *buffer = calloc(sb->length, sizeof(dtype)) ;
      memcpy(buffer, (double *)sb->content, sb->length*sizeof(dtype)) ;

      }
    stream_free_block(sb) ;
    }
  stream_free_data(strm) ;
  }


/*
  def put_data(self, uri, timeseries):
  #-----------------------------------
    stream = None
    try:
      stream = WebStreamWriter(self._sd_uri)
      MAXPOINTS = 50000   ##### TESTING    (200K bytes if double precision)
      params = { }
      if getattr(timeseries,'rate', None):
        params['rate'] = timeseries.rate
      pos = 0
      count = len(timeseries)
      while count > 0:
        blen = min(count, MAXPOINTS)
        if getattr(timeseries, 'clock', None):
          params['clock'] = timeseries.clock[pos:pos+blen]
        stream.write_block(SignalData(uri, timeseries.time[pos], timeseries.data[pos:pos+blen], **params))
        pos += blen
        count -= blen
    except Exception, msg:
      logging.error('Error in stream: %s', msg)
      raise
    finally:
      if stream: stream.close()


    return rec

**/
