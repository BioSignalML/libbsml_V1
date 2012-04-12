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
#include <uriparser/Uri.h>

#include "bsml_repository.h"
#include "bsml_recording.h"
#include "bsml_internal.h"
#include "bsml_stream.h"
#include "bsml_data.h"
#include "utility/bsml_string.h"
#include "utility/bsml_json.h"


typedef enum {
  HTTP_GET = 1,
  HTTP_PUT,
  HTTP_POST,
  HTTP_DELETE
  } HTTP_METHOD ;



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
  // First parse URI to check validity and to get host and port components
  UriParserStateA parse_state ;
  UriUriA parsed_uri ;
  parse_state.uri = &parsed_uri ;
  if (uriParseUriA(&parse_state, uri) != URI_SUCCESS || parsed_uri.hostText.first == NULL) {
    uriFreeUriMembersA(&parsed_uri);
    return NULL ;
    }
  int hostlen = parsed_uri.hostText.afterLast - parsed_uri.hostText.first ;
  int port ;
  if (parsed_uri.portText.first != NULL) sscanf(parsed_uri.portText.first, "%d", &port) ;
  else port = 80 ;
  uriFreeUriMembersA(&parsed_uri);
  bsml_repository *repo = ALLOCATE(bsml_repository) ;
  if (repo) {
    repo->graph = bsml_repository_get_metadata(repo, "") ;
    repo->uri = bsml_string_copy(uri) ;
    repo->host = bsml_string_copy(parsed_uri.hostText.first) ;
    ((char *)repo->host)[hostlen] = '\0' ;
    repo->port = port ;
    repo->metadata_end = bsml_string_cat(uri, BSML_REPOSITORY_RECORDING) ;
    }
  return repo ;
  }


void bsml_repository_close(bsml_repository *repo)
/*=============================================*/
{
  if (repo) {
    bsml_rdfgraph_free(repo->graph) ;
    bsml_string_free(repo->uri) ;
    bsml_string_free(repo->host) ;
    bsml_string_free(repo->metadata_end) ;
    free(repo) ;
    }
  }


bsml_rdfgraph *bsml_repository_get_metadata(bsml_repository *repo, const char *uri)
/*================================================================================*/
{
  if (repo) {
    const char *md_uri = bsml_string_cat(repo->metadata_end, uri) ;
    if (md_uri) {
      bsml_rdfgraph *graph = bsml_rdfgraph_create_and_load_rdf(uri, md_uri) ;
      bsml_string_free(md_uri) ;
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
    const char *md_uri = bsml_string_cat(repo->metadata_end, uri) ;
    curl_easy_setopt(curl, CURLOPT_URL, md_uri) ;

    struct curl_slist *slist = NULL ;
    const char *ctype = bsml_string_cat("Content-Type: ", MIMETYPE_RDFXML) ;
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
    bsml_string_free(ctype) ;
    bsml_string_free(md_uri) ;
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


void bsml_repository_get_timeseries(bsml_repository *repo, const char *data_uri,
/*============================================================================*/
                                     double start, double duration, TimeseriesProcess process)
{
  bsml_stream_data *strm = bsml_stream_data_request(repo->host, repo->port,
    BSML_REPOSITORY_STREAM, data_uri, start, duration, bsml_stream_double_type) ;

  bsml_stream_block *sb ;

  while ((sb = bsml_stream_data_read(strm)) != NULL) {
    switch (sb->type) {
     case BSML_STREAM_ERROR_BLOCK:
      bsml_log_error("STREAM ERROR: %-*s\n", sb->length, sb->content) ;
      break ;

     case BSML_STREAM_DATA_BLOCK:
      if (strcmp(bsml_json_get_string(sb->header, "dtype"), bsml_stream_double_type))
        bsml_log_error("ERROR: Stream returned wrong type of data\n") ;
      else {
        const char *ctype = bsml_json_get_string(sb->header, "ctype") ;
        if (ctype && strcmp(ctype, bsml_stream_double_type))
          bsml_log_error("ERROR: Stream returned wrong type of timing data\n") ;
        else {
          int count = bsml_json_get_integer(sb->header, "count") ;
          int len = sb->length/sizeof(double) ;
          if (ctype) len /= 2 ;
          if (count != len)
            bsml_log_error("ERROR: Stream returned wrong amount of data\n") ;
          else {
            double *data = (double *)sb->content ;
            bsml_timeseries *ts = bsml_timeseries_alloc(
              bsml_json_get_string(sb->header, "uri"),
              bsml_time_from_seconds(bsml_json_get_number(sb->header, "start")),
              count,
              ctype ? 0.0 : bsml_json_get_number(sb->header, "rate"),
              ctype ? data : NULL,
              ctype ? data + count : data) ;
            process(ts) ;
            bsml_timeseries_free(ts) ;
            }
          }
        }
      break ;

     default:
      bsml_log_error("STREAM UNKNOWN BLOCK: %c\n", sb->type) ;
      break ;
      }
    bsml_stream_block_free(sb) ;
    }
  if (strm->error != BSML_STREAM_ERROR_NONE)  // Always check for errors...
    bsml_log_error("STREAM ERROR %d: %s\n", strm->error, bsml_stream_error_text(strm->error)) ;
  bsml_stream_data_free(strm) ;
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

#ifdef TEST_REPOSITORY

#define REPOSITORY  "http://devel.biosignalml.org"

//#define TEST_SIGNAL "http://devel.biosignalml.org/recording/test/sw4"
//#define TEST_SIGNAL "http://devel.biosignalml.org/recording/test/ecgca102I"
//#define TEST_SIGNAL "http://example.org/test/xx/ecg102"
#define TEST_SIGNAL "http://example.org/test/xx/ecg102/signal/3"




void print_data(bsml_timeseries *d)
/*===============================*/
{
  printf("RCVD %d data values from %s starting at %g at rate %g\n",
    d->length, d->uri, bsml_time_as_seconds(d->start), d->rate) ;

  int l = d->length ;
  double *dp = d->data ;
  while (l > 0) {
    printf("%f\n", *dp) ;
    ++dp ;
    --l ;
    }

  }


int main(void)
/*==========*/
{
  bsml_initialise() ;

  bsml_repository *repo = bsml_repository_connect(REPOSITORY) ;

  double t = 0.0 ;
  double d = 0.1 ;
  while (t < 1.0) {
    bsml_repository_get_timeseries(repo, TEST_SIGNAL, t, d, print_data) ;
    t += d ;
    }

  bsml_repository_close(repo) ;

  bsml_finish() ;
  }

#endif
