/******************************************************
 *
 *  BioSignalML Project API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 ******************************************************
 */

#ifndef _SSF_H
#define _SSF_H

#include "md5/md5.h"
#include "cJSON/cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif


#define STREAM_BUFFER_SIZE 4096

typedef enum {
  STREAM_ERROR_UNEXPECTED_TRAILER = 1,
  STREAM_ERROR_MISSING_HEADER_LF,
  STREAM_ERROR_MISSING_TRAILER,
  STREAM_ERROR_INVALID_CHECKSUM,
  STREAM_ERROR_MISSING_TRAILER_LF,
  STREAM_ERROR_HASHRESERVED,
  STREAM_ERROR_WRITEOF
  } STREAM_ERROR ;

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
  STREAM_STATE_HDRLEN,
  STREAM_STATE_HEADER,
  STREAM_STATE_HDREND,
  STREAM_STATE_CONTENT,
  STREAM_STATE_TRAILER,
  STREAM_STATE_CHECKSUM,
  STREAM_STATE_CHECKDATA,
  STREAM_STATE_BLOCKEND,
  STREAM_STATE_BLOCK
  } STREAM_STATE ;


typedef struct {
  int number ;
  char type ;
  cJSON *header ;
  int length ;
  char *content ;
  STREAM_ERROR error ;
  } StreamBlock ;


typedef struct {
  int file ;
  STREAM_CHECKSUM checksum ;
  StreamBlock block ;

  STREAM_STATE state ;
  int datalen ;
  int expected ;
  char *databuf ;
  char *datapos ;
  char *storepos ;

  char *jsonhdr ;

	md5_state_t md5 ;
  char checktext[33] ;

  } StreamReader ;


typedef struct {
  int file ;
  STREAM_CHECKSUM checksum ;
  } StreamWriter ;


char *stream_error_text(STREAM_ERROR) ;

StreamBlock *stream_new_block(void) ;
StreamBlock *stream_dup_block(StreamBlock *) ;
void stream_free_block(StreamBlock *) ;

StreamReader *stream_new_input(STREAM_CHECKSUM) ;
StreamReader *stream_new_reader(int, STREAM_CHECKSUM) ;
void stream_free_input(StreamReader *) ;
void stream_free_reader(StreamReader *) ;
void stream_process_data(StreamReader *) ;
StreamBlock *stream_read_block(StreamReader *) ;


StreamWriter *stream_new_writer(int, STREAM_CHECKSUM) ;
void stream_free_writer(StreamWriter *) ;
int stream_write_block(StreamWriter *, StreamBlock *, STREAM_CHECKSUM) ;


/***
typedef struct {


  } SimpleStream ;


SimpleStream *ssf_new_reader(int fh) ;

SimpleStream *ssf_new_writer(int fh) ;

***/


#ifdef __cplusplus
} ;
#endif

#endif
