#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


#include "ssf.h"



int main(void)
/*==========*/
{

  

  int infile = fileno(stdin) ;
//               open("/Users/dave/biosignalml/libbsml/src/ssf/t", O_RDONLY) ;

  StreamReader *input = stream_new_reader(infile, STREAM_CHECKSUM_CHECK) ;
  StreamWriter *out = stream_new_writer(fileno(stdout), STREAM_CHECKSUM_STRICT) ;

  StreamBlock *block ;

  if (input == NULL) {
    printf("Cannot open stream...") ;
    exit(1) ;
    }

  while (1) {
    block = stream_read_block(input) ;

    if (input->state < 0) break ; // EOF

    else if (block) {

      if (block->error)
        fprintf(stderr, "%d ERROR %d (%s)\n", block->number, block->error,
                                              stream_error_text(block->error)) ;
/*    else {
      char *json = cJSON_PrintUnformatted(block->header) ;
      printf("%d BLOCK: Type=%c, Header=%s, Length=%d\n",
        block->number, block->type, json, block->length) ;
      free(json) ;


      }
*/
      stream_write_block(out, block, STREAM_CHECKSUM_INHERIT) ;

      stream_free_block(block) ;

      }

    }

  stream_free_reader(input) ;
  stream_free_writer(out) ;
  }


/*

  SimpleStream *writer = ssf_new_writer(fileno(stdout))


  ssf_writer


  init...

    uri (of stream (= Recording's))

    (number of channels ?)

    each signal's metadata
      ssf_writer_add_signal(writer, sig)



    ssf_writer_write_data(writer, data)

      when does block get written??
      how is data for different signals placed in a block?
      
      Or do we have a format of DATA_FRAMED (that has channels=N attribute)??

      write_data_block(


      datablockwriter = 
      write_data_frame


*/
