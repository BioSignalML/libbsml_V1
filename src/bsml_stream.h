/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/
/** @file
  * Websocket streams.
  */

#ifndef _BSML_STREAM_H
#define _BSML_STREAM_H

#include <pthread.h>
#include <libwebsockets.h>
#include <jansson.h>

#include "utility/bsml_queue.h"


#define BSML_STREAM_PROTOCOL "biosignalml-ssf"

#define BSML_STREAM_VERSION  1


#ifdef __cplusplus
extern "C" {
#endif


#define STREAM_DOUBLE "f8"


typedef char BSML_STREAM_BLOCK_TYPE ;

#define BSML_STREAM_DATA_REQUEST 'd'
#define BSML_STREAM_DATA_BLOCK   'D'
#define BSML_STREAM_INFO_BLOCK   'I'
#define BSML_STREAM_RDF_BLOCK    'R'
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
  BSML_STREAM_ERROR_MEMORY,
  BSML_STREAM_ERROR_QUEUE,
  } BSML_STREAM_ERROR_CODE ;

typedef enum {
  BSML_STREAM_CHECKSUM_INHERIT = 0,
  BSML_STREAM_CHECKSUM_STRICT,
  BSML_STREAM_CHECKSUM_CHECK,
  BSML_STREAM_CHECKSUM_IGNORE,
  BSML_STREAM_CHECKSUM_NONE
  } BSML_STREAM_CHECKSUM ;

typedef enum {
  BSML_STREAM_STARTING = 0,
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
  } bsml_streamblock ;


typedef struct bsml_StreamReader
  bsml_streamreader ;

typedef struct bsml_streamblock_Queue
  bsml_streamblock_queue ;

typedef struct {
  const char *uri ;
  double start ;
  double duration ;
  const char *dtype ;
  //  offset, count
  int maxsize ;
  BSML_STREAM_STATE state ;
  BSML_STREAM_ERROR_CODE error ;
  int stopped ;
  pthread_t thread ;
  bsml_queue *blockQ ;
  bsml_streamreader *sp ;
  struct libwebsocket *ws ;
  } bsml_streamdata ;


extern const char *bsml_stream_double_type ;  //!< The datatype of a double on this platform
extern const char *bsml_stream_long_type ;    //!< The datatype of a long on this platform

/** Initialise globals used for stream connections.
  */
void bsml_stream_initialise(void) ;

/** Finished with globals used for stream connections.
  */
void bsml_stream_finish(void) ;

/** Return human readable text for an error.
  */
const char *bsml_stream_error_text(BSML_STREAM_ERROR_CODE code) ;

bsml_streamdata *bsml_streamdata_request(const char *uri, double start, double duration,
                                                          const char *dtype, int maxsize) ;

// Units...

void bsml_streamdata_free(bsml_streamdata *streamdata) ;

bsml_streamblock *bsml_streamdata_read(bsml_streamdata *streamdata) ;
void bsml_streamblock_free(bsml_streamblock *sb) ;

#ifdef __cplusplus
  } ;
#endif

#endif
