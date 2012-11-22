#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "print_json.h"
#include "bsml_stream.h"

#define STREAM_ENDPOINT "/stream/data/"


int main(void)
/*==========*/
{
  const char *recording = "http://devel.biosignalml.org/resource/physiobank/mitdb/102" ;
  double start = 0.0 ;
  double duration = 1800.0 ;

  int CHUNKSIZE = 10000 ;

  int channels = 1 ;
  json_t *uris = NULL ;

  const char *DATAFORMAT = STREAM_DOUBLE ;

  double *data = NULL ;      // Curerently assuming everything is at the same rate....


  bsml_stream_initialise() ;

  // Setup stream, set options (e.g. block size), add signals, etc,
  // then bsml_stream_start(strm)

  bsml_stream *strm = bsml_stream_create(recording, start, duration, DATAFORMAT) ;
  bsml_stream_set_maxsize(strm, CHUNKSIZE) ;

  bsml_stream_request(strm, "devel.biosignalml.org", 80, STREAM_ENDPOINT) ;

  bsml_stream_block *sb ;
  
  fprintf(stderr, "Start state: %d\n", strm->state) ;

  int frameno = 0 ;
  while ((sb = bsml_stream_read(strm)) != NULL) {

    fprintf(stderr, "Block %c: %d\n  ", sb->type, sb->length) ;
    print_json_line(sb->header, stderr) ;

    if      (sb->type == BSML_STREAM_ERROR_BLOCK) {
      fprintf(stderr, "ERROR: %-*s\n", sb->length, sb->content) ;
      }

    else if (sb->type == BSML_STREAM_INFO_BLOCK) {
      if (uris) json_decref(uris) ;
      uris = json_object_get(sb->header, "uris") ;
      if (uris && json_is_array(uris)) {
        size_t n = json_array_size(uris) ;
        json_incref(uris) ;
        if (channels != n) {
          channels = n ;
          if (data) free(data) ;
          data = calloc(sizeof(double), channels*CHUNKSIZE) ;
          }
        }
      }

    else if (sb->type == BSML_STREAM_RDF_BLOCK) {

      }

    else if (sb->type == BSML_STREAM_DATA_BLOCK) {
      json_t *info = json_object_get(sb->header, "info") ;
      if (info && json_is_integer(info)) {
        int chan = json_integer_value(info) ;
        double *dp = data + chan ;
        double *cp = (double *)sb->content ;
        int l = sb->length/sizeof(double) ;
        while (l > 0) {
          *dp = *cp ;
          dp += channels ;
          ++cp ;
          --l ;
          }
        if (chan == (channels-1)) {
          double *dp = data ;
          int i ;
          l = sb->length/sizeof(double) ;
          while (l > 0) {
            printf("%d ", frameno) ;
            for (i = 0 ;  i < channels ;  ++i) {
              if (i > 0) printf(" ") ;
              printf("%f", *dp) ;
              ++dp ;
              }
            printf("\n") ;
            ++frameno ;
            --l ;
            }
          }
        }
      // get dtype from sb->header
      // JSON *header ;
//      dtype *buffer = calloc(sb->length, sizeof(dtype)) ;
//      memcpy(buffer, (double *)sb->content, sb->length*sizeof(dtype)) ;
      }
    bsml_stream_block_free(sb) ;
    }

  if (strm->error != BSML_STREAM_ERROR_NONE)  // Always check for errors...
    fprintf(stderr, "ERROR %d: %s\n", strm->error, bsml_stream_error_text(strm->error)) ;

  fprintf(stderr, "End state: %d\n", strm->state) ;


  bsml_stream_free(strm) ;
  if (data) free(data) ;
  if (uris) json_decref(uris) ;
  bsml_stream_finish() ;
  }
