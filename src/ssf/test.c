#include <stdio.h>
#include <stdlib.h>

#include "ssf.h"



int main(void)
/*==========*/
{
  StreamReader *input = stream_new_reader(fileno(stdin), STREAM_CHECKSUM_CHECK) ;

  if (input == NULL) {
    printf("Cannot open stream...") ;
    exit(1) ;
    }

  while (stream_read_block(input)) {

    if (input->error)
      printf("ERROR %d in BLOCK %d (%s)\n", input->blockno, input->error,
                                            stream_error_text(input->error)) ;
    else
      printf("%d BLOCK: Type=%c, Header=%s, Length=%d",
        input->blockno, input->type, cJSON_Print(input->header), input->length) ;
    }

  stream_free_reader(input) ;
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
