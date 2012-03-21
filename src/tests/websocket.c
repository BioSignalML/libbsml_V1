#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#include "bsml-stream.h"

#define REPOSITORY_STREAM "/stream/data/"


const char *string_copy(const char *s)
/*==================================*/
{
  if (s) {
    int l = strlen(s) + 1 ;
    char *t = malloc(l) ;
    memcpy(t, s, l) ;
    return t ;
    }
  else return NULL ;
  }


int main(void)
/*==========*/
{

  stream_initialise() ;

  stream_data *strm = stream_data_request("localhost", 80,
    REPOSITORY_STREAM, "http://devel.biosignalml.org/test/sinewave6", 0.0, 1.0) ;

  stream_block *sb ;
  
  printf("%d, %lx\n", strm->state, strm->block) ;

  while ((sb = stream_data_read(strm)) != NULL) {
    printf("Block %c: %d\n", sb->type, sb->length) ;
    if      (sb->type == ERROR_BLOCK) {
      printf("ERROR: %-*s\n", sb->length, sb->content) ;
      }

    else if (sb->type == DATA_BLOCK) {
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
    stream_free_block(sb) ;
    }

  if (strm->error != STREAM_ERROR_NONE)  // Always check for errors...
    printf("ERROR %d: %s\n", strm->error, stream_error_text(strm->error)) ;

  printf("%d, %lx\n", strm->state, strm->block) ;

  stream_free_data(strm) ;


  stream_finish() ;

  }
