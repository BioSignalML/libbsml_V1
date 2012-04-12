#ifndef _BSML_STREAM_H
#define _BSML_STREAM_H

#include <libwebsockets.h>
#include <jansson.h>

#include "utility/bsml_queue.h"


#define BSML_STREAM_PROTOCOL "biosignalml-ssf"

#define BSML_STREAM_VERSION  1


#ifdef __cplusplus
extern "C" {
#endif

typedef char BSML_STREAM_BLOCK_TYPE ;

#define BSML_STREAM_DATA_REQUEST 'd'
#define BSML_STREAM_DATA_BLOCK   'D'
#define BSML_STREAM_ERROR_BLOCK  'E'


typedef enum {
  BSML_STREAM_ERROR_NONE = 0,
  BSML_STREAM_ERROR_UNEXPECTED_TRAILER,
  BSML_STREAM_ERROR_MISSING_HEADER_LF,
  BSML_STREAM_ERROR_MISSING_TRAILER,
  BSML_STREAM_ERROR_INVALID_CHECKSUM,
  BSML_STREAM_ERROR_MISSING_TRAILER_LF,
  BSML_STREAM_ERROR_HASHRESERVED,
  BSML_STREAM_ERROR_WRITEOF,
  BSML_STREAM_ERROR_VERSION_MISMATCH,
  BSML_STREAM_ERROR_BAD_JSON_HEADER,
  BSML_STREAM_ERROR_BAD_FORMAT,
  BSML_STREAM_ERROR_NO_CONNECTION,
  } BSML_STREAM_ERROR_CODE ;

typedef enum {
  BSML_STREAM_CHECKSUM_INHERIT = 0,
  BSML_STREAM_CHECKSUM_STRICT,
  BSML_STREAM_CHECKSUM_CHECK,
  BSML_STREAM_CHECKSUM_IGNORE,
  BSML_STREAM_CHECKSUM_NONE
  } BSML_STREAM_CHECKSUM ;

typedef enum {
  BSML_STREAM_STARTING = -1,
  BSML_STREAM_OPENED,
  BSML_STREAM_RUNNING,
  BSML_STREAM_ERROR,
  BSML_STREAM_CLOSED
  } BSML_STREAM_STATE ;


typedef struct {
  int number ;
  BSML_STREAM_BLOCK_TYPE type ;
  json_t *header ;
  int length ;
  char *content ;
  } bsml_stream_block ;


typedef struct bsml_Stream_Reader
  bsml_stream_reader ;

typedef struct bsml_stream_block_Queue
  bsml_stream_block_queue ;

typedef struct {
  const char *uri ;
  double start ;
  double duration ;
  const char *dtype ;
  //  offset, count, maxsize
  BSML_STREAM_STATE state ;
  BSML_STREAM_ERROR_CODE error ;
  int stopped ;
  bsml_queue *blockQ ;
  bsml_stream_reader *sp ;
  struct libwebsocket *ws ;
  } bsml_stream_data ;


extern const char *bsml_stream_double_type ;  //!< The datatype of a double on this platform
extern const char *bsml_stream_long_type ;    //!< The datatype of a long on this platform
void bsml_stream_initialise(void) ;

void bsml_stream_finish(void) ;

const char *bsml_stream_error_text(BSML_STREAM_ERROR_CODE code) ;


bsml_stream_data *bsml_stream_data_request(const char *host, int port, const char *endpoint,
                                           const char *uri, double start, double duration, const char *dtype) ;

bsml_stream_data *bsml_stream_data_new(const char *uri) ;

void bsml_stream_data_free(bsml_stream_data *sd) ;


bsml_stream_block *bsml_stream_data_read(bsml_stream_data *sd) ;

void bsml_stream_block_free(bsml_stream_block *sb) ;

#ifdef __cplusplus
  } ;
#endif

#endif
