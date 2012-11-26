#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "bsml_stream.h"

#define STREAM_ENDPOINT "/stream/data/"


void get_data(const char *recording, double start, double duration)
/*===============================================================*/
{
  int CHUNKSIZE = 10000 ;

  int channels = 1 ;
  json_t *siguris = NULL ;

  const char *DATAFORMAT = STREAM_DOUBLE ;

  double *data = NULL ;      // Curerently assuming everything is at the same rate....


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
    json_dumpf(sb->header, stderr, JSON_ENSURE_ASCII) ;

    if      (sb->type == BSML_STREAM_ERROR_BLOCK) {
      fprintf(stderr, "ERROR: %-*s\n", sb->length, sb->content) ;
      }

    else if (sb->type == BSML_STREAM_INFO_BLOCK) {
      if (siguris) json_decref(siguris) ;
      siguris = json_object_get(sb->header, "signals") ;
      // Also have "channels" field (plus "rates" and "units").
      if (siguris && json_is_array(siguris)) {
        size_t n = json_array_size(siguris) ;
        json_incref(siguris) ;
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
      // Need to check rates match...
      // Or have rates in Info block and check when it's received...
      // What about signals where the rate changes??
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
  if (siguris) json_decref(siguris) ;
  }



void send_data(const char *recording, int channels, double rate) // From stdin
/*============================================================*/
{

  // Need to first put recording into repository...

  // Info block with recording, (signals), channels, rates, units
  }


int main(void)
/*==========*/
{
  bsml_stream_initialise() ;


  get_data("http://devel.biosignalml.org/resource/physiobank/mitdb/102", 0.0, 0.05) ;


  bsml_stream_finish() ;
  }

