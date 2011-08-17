#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "ssf.h"
#include "dictionary.h"

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


int main(void)
//============
{

  StreamReader *stream = stream_new_input(STREAM_CHECKSUM_CHECK) ;
  StreamWriter *out = stream_new_writer(fileno(stdout), STREAM_CHECKSUM_STRICT) ;

  curl_global_init(CURL_GLOBAL_ALL) ;

  CURL *link = curl_easy_init() ;

  curl_easy_setopt(link, CURLOPT_URL,
    "http://devel.biosignalml.org/recording/sinetest1.edf/signal/0") ;

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

//            cJSON *lenp = cJSON_GetObjectItem(block->header, "length") ;
//            if (lenp && lenp->type == cJSON_Number) block->length = lenp->valueint ;

    if (block->type == 'D') {
      dict *d = cJSON_dictionary(block->header) ;
      dict_print(d) ;
      dict_free(d) ;


      printf("Len: %d\n", block->length) ;
      
      double *v = (double *)block->content ;
      int l = block->length/8 ;
      while (l-- > 0) {
        printf("%g ", *v) ;
        ++v ;
        }
      printf("\n") ;
      }

//    stream_write_block(out, block, STREAM_CHECKSUM_INHERIT) ;
    }

  stream_free_input(stream) ;
  stream_free_writer(out) ;
  
  curl_global_cleanup() ;

  }

