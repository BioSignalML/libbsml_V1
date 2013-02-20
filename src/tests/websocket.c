#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

#include "bsml_stream.h"



void get_data(const char *recording, double start, double duration)
/*===============================================================*/
{
  int CHUNKSIZE = 10000 ;

  double *data = NULL ;
  int channels = 0 ;
  json_t *siguris = NULL ;

  const char *DATAFORMAT = STREAM_DOUBLE ;


  // Setup stream, set options (e.g. block size), add signals, etc,
  // then bsml_stream_start(strm)
  bsml_streamdata *strm = bsml_streamdata_request(recording, start, duration, DATAFORMAT, CHUNKSIZE) ;

  bsml_streamblock *sb ;
  int frameno = 0 ;

  while ((sb = bsml_streamdata_read(strm)) != NULL) {
#ifdef LOG_HEADER
    fprintf(stderr, "%s Block %c: %d\n  ", run_id, sb->type, sb->length) ;
    json_dumpf(sb->header, stderr, JSON_ENSURE_ASCII) ;
    fprintf(stderr, "\n") ;
#endif
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
              printf("%8g", *dp) ;
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
    bsml_streamblock_free(sb) ;
    }

  if (strm->error != BSML_STREAM_ERROR_NONE)  // Always check for errors...
    fprintf(stderr, "ERROR %d: %s\n", strm->error, bsml_stream_error_text(strm->error)) ;


  bsml_streamdata_free(strm) ;
  if (data) free(data) ;
  if (siguris) json_decref(siguris) ;
  }



void send_data(const char *recording, int channels, double rate) // From stdin
/*============================================================*/
{

  // Need to first put recording into repository...

  // Info block with recording, (signals), channels, rates, units
  }


int main(int argc, char **argv)
/*===========================*/
{
  if (argc < 2) {
    printf("Usage: %s SIGNAL_URI\n", argv[0]) ;
    exit(1) ;
    }


  bsml_stream_initialise() ;
  get_data(argv[1], 0.0, 1806.0) ;
  bsml_stream_finish() ;
  }

