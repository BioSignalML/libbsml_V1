#ifndef _BSML_STREAM_H
#define _BSML_STREAM_H

#include <libwebsockets.h>
#include <jansson.h>


#define STREAM_PROTOCOL "biosignalml-ssf"

#define STREAM_VERSION  1


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
  STREAM_ERROR_NONE = 0,
  STREAM_ERROR_UNEXPECTED_TRAILER,
  STREAM_ERROR_MISSING_HEADER_LF,
  STREAM_ERROR_MISSING_TRAILER,
  STREAM_ERROR_INVALID_CHECKSUM,
  STREAM_ERROR_MISSING_TRAILER_LF,
  STREAM_ERROR_HASHRESERVED,
  STREAM_ERROR_WRITEOF,
  STREAM_ERROR_VERSION_MISMATCH,
  STREAM_ERROR_INVALID_MORE_FLAG,
  STREAM_ERROR_BAD_JSON_HEADER,
  } STREAM_ERROR_CODE ;

typedef enum {
  STREAM_CHECKSUM_INHERIT = 0,
  STREAM_CHECKSUM_STRICT,
  STREAM_CHECKSUM_CHECK,
  STREAM_CHECKSUM_IGNORE,
  STREAM_CHECKSUM_NONE
  } STREAM_CHECKSUM ;

typedef enum {
  STREAM_STATE_ENDED = -1,
  STREAM_STATE_RESET,
  STREAM_STATE_TYPE,
  STREAM_STATE_VERSION,
  STREAM_STATE_MORE,
  STREAM_STATE_HDRLEN,
  STREAM_STATE_HEADER,
  STREAM_STATE_DATALEN,
  STREAM_STATE_HDREND,
  STREAM_STATE_CONTENT,
  STREAM_STATE_TRAILER,
  STREAM_STATE_CHECKSUM,
  STREAM_STATE_CHECKDATA,
  STREAM_STATE_BLOCKEND,
  STREAM_STATE_BLOCK
  } STREAM_READER_STATE ;


typedef struct {
  int number ;
  char type ;
  json_t *header ;
  int length ;
  char *content ;
  } stream_block ;


typedef struct {

  STREAM_CHECKSUM checksum ;
  STREAM_READER_STATE state ;
  STREAM_ERROR_CODE error ;

  stream_block *block ;
  int number ;

  int version ;
  int more ;
  int expected ;
  char *storepos ;

  char *jsonhdr ;

	md5_state_t md5 ;
  char checktext[33] ;

  } stream_reader ;


typedef enum {
  STREAM_STARTING = -1,
  STREAM_OPENED,
  STREAM_RUNNING,
  STREAM_ERROR,
  STREAM_CLOSED
  } STREAM_STATE ;


typedef struct {
  char *uri ;
  double start ;
  double duration ;

  //  offset, count, maxsize

  STREAM_STATE state ;
  STREAM_ERROR_CODE error ;
  int stopped ;
  stream_block *block ;
  stream_reader *sp ;
  struct libwebsocket *ws ;
  } stream_data ;


void stream_initialise(void) ;

void stream_finish(void) ;

const char *stream_error_text(STREAM_ERROR_CODE code) ;

stream_data *stream_data_new(const char *uri) ;

stream_data *stream_data_request(const char *host, int port, const char *endpoint,
                                 const char *uri, double start, double duration) ;

void stream_data_free(stream_data *sd) ;

stream_block *stream_data_read(stream_data *sd) ;


#ifdef __cplusplus
  } ;
#endif

#endif
