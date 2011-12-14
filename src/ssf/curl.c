#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "ssf.h"
#include "dictionary.h"
#include "bsml_model.h"


dict *cJSON_dictionary(cJSON *item) ;


static size_t stream_processor(void *data, size_t size, size_t len, void *ud)
//===========================================================================
{
  StreamReader *sp = (StreamReader *)ud ;
  size_t datalen = size * len ;
  if (datalen > 0 && sp->state == STREAM_STATE_BLOCK) return 0 ;
  sp->databuf = data ;
  sp->datapos = sp->databuf ;
  sp->datalen = datalen ;
  stream_process_data(sp) ;  // returns needing data, or block error, or have good block
  return (datalen - sp->datalen) ;
  }


int bsml_stream_read_timeseries(bsml_timeseries *ts, const char *uri, double start, double duration)
//==================================================================================================
{
printf("Reading %s (%g : %g)\n", uri, start, duration) ;
  int result = 1 ;
  StreamReader *stream = stream_new_input(STREAM_CHECKSUM_CHECK) ;
  CURL *link = curl_easy_init() ;
  char *fulluri = NULL ;
  if (start >= 0.0 && duration > 0.0) {
    asprintf(&fulluri, "%s?%g:%g", uri, start, duration) ;
    curl_easy_setopt(link, CURLOPT_URL, fulluri) ;
    free(fulluri) ;
    }
  else curl_easy_setopt(link, CURLOPT_URL, uri) ;
  struct curl_slist *headers = curl_slist_append(NULL, "Accept: application/x-stream" ) ;
  curl_easy_setopt(link, CURLOPT_HTTPHEADER, headers) ;
  curl_easy_setopt(link, CURLOPT_WRITEFUNCTION, stream_processor) ;
  curl_easy_setopt(link, CURLOPT_WRITEDATA, (void *)stream) ;
  curl_easy_setopt(link, CURLOPT_USERAGENT, "biosignalml-client/1.0") ;
  curl_easy_setopt(link, CURLOPT_NOPROGRESS, 1L) ;
  curl_easy_perform(link) ;
  curl_easy_cleanup(link) ;
  curl_slist_free_all(headers) ;

  if (stream->state == STREAM_STATE_BLOCK) {
    StreamBlock *block = &(stream->block) ;
    if (block->error)
      fprintf(stderr, "%d ERROR %d (%s)\n", block->number, block->error,
                                            stream_error_text(block->error)) ;
    if (block->type == 'D') {

      if (ts->data) free(ts->data) ;
      ts->data = malloc(block->length) ;
      memcpy(ts->data, block->content, block->length) ;

      dict *header = cJSON_dictionary(block->header) ;
      ts->offset = dict_get_integer(header, "startpos") ;
    // Check rate etc rate matches...
      ts->period = dict_get_real(header, "period") ;
      ts->rate = dict_get_real(header, "rate") ;

      const char *dt = dict_get_string(header, "datatype") ;
      int size = atoi(dt+2) ;
      ts->len = block->length/size ;

      // Check size and *dt for endianess correction...

      dict_free(header) ;
      result = 0 ;
      }
    }

  stream_free_input(stream) ;
  return result ;
  }



void bsml_stream_initialise()
//===========================
{
  curl_global_init(CURL_GLOBAL_ALL) ;
  }



void bsml_stream_finish()
//=======================
{
  curl_global_cleanup() ;
  }


#ifdef TEST_CURL

int main(void)
//============
{

  bsml_stream_initialise() ;

  bsml_timeseries *ts = bsml_stream_read_data(
                          "http://devel.biosignalml.org/recording/sinetest1.edf/signal/0",
                          0.0, 0.1) ;



//    stream_write_block(out, block, STREAM_CHECKSUM_INHERIT) ;

  bsml_timeseries_free(ts) ;

  bsml_stream_finish() ;

  }
#endif
