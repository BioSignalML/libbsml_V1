#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#include "bsml_stream.h"

#define REPOSITORY_STREAM "/stream/data/"


int main(void)
/*==========*/
{

  bsml_stream_initialise() ;

  bsml_stream_data *strm = bsml_stream_data_request("localhost", 80,
    REPOSITORY_STREAM, "http://devel.biosignalml.org/recording/test/sw4", 0.0, 1.0) ;

  bsml_stream_block *sb ;
  
  printf("%d, %lx\n", strm->state, (long)strm->block) ;

  while ((sb = bsml_stream_data_read(strm)) != NULL) {
    printf("Block %c: %d\n", sb->type, sb->length) ;
    if      (sb->type == BSML_STREAM_ERROR_BLOCK) {
      printf("ERROR: %-*s\n", sb->length, sb->content) ;
      }

    else if (sb->type == BSML_STREAM_DATA_BLOCK) {
      double *dp = (double *)sb->content ;
      int l = sb->length/sizeof(double) ;

      while (l > 0) {
        printf("%f\n", *dp) ;
        ++dp ;
        --l ;
        }


      // get dtype from sb->header
      // JSON *header ;

//      dtype *buffer = calloc(sb->length, sizeof(dtype)) ;
//      memcpy(buffer, (double *)sb->content, sb->length*sizeof(dtype)) ;

      }
    bsml_stream_free_block(sb) ;
    }

  if (strm->error != BSML_STREAM_ERROR_NONE)  // Always check for errors...
    printf("ERROR %d: %s\n", strm->error, bsml_stream_error_text(strm->error)) ;

  printf("%d, %lx\n", strm->state, (long)strm->block) ;

  bsml_stream_free_data(strm) ;


  bsml_stream_finish() ;

  }
