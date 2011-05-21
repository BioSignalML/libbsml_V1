/******************************************************
 *
 *  BioSignalML Project API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $Id$
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


#define STREAM_BUFFER_SIZE              4096

#define STREAM_ERROR_UNEXPECTED_TRAILER    1
#define STREAM_ERROR_MISSING_HEADER_LF     2
#define STREAM_ERROR_MISSING_TRAILER       3
#define STREAM_ERROR_INVALID_CHECKSUM      4
#define STREAM_ERROR_MISSING_TRAILER_LF    5
#define STREAM_ERROR_HASHRESERVED          6
#define STREAM_ERROR_WRITEOF               7

#define STREAM_CHECKSUM_STRICT             1
#define STREAM_CHECKSUM_CHECK              2
#define STREAM_CHECKSUM_IGNORE             3
#define STREAM_CHECKSUM_NONE               4


typedef struct {
  int number ;
  char type ;
  cJSON *header ;
  int length ;
  char *content ;
  int error ;
  } StreamBlock ;


typedef struct {
  int file ;
  int checksum ;
  StreamBlock block ;

  int state ;
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
  int checksum ;
  } StreamWriter ;


char *stream_error_text(int) ;

StreamBlock *stream_new_block(void) ;
StreamBlock *stream_dup_block(StreamBlock *) ;
void stream_free_block(StreamBlock *) ;

StreamReader *stream_new_reader(int, int) ;
void stream_free_reader(StreamReader *) ;
StreamBlock *stream_read_block(StreamReader *) ;


StreamWriter *stream_new_writer(int, int) ;
void stream_free_writer(StreamWriter *) ;
int stream_write_block(StreamWriter *, StreamBlock *, int) ;


/****
typedef struct {
  char *uri ;
  } Recording ;

typedef struct {
  Recording *recording ;
  int number ;
  double rate ;
  char *label ;
  } Signal ;

typedef struct {
  Signal *signal ;
  double starttime ;
  double duration ;
  int length ;
  double *data ;
  } SignalData ;


typedef struct {


  } SimpleStream ;


SimpleStream *ssf_new_reader(int fh) ;

SimpleStream *ssf_new_writer(int fh) ;

***/


#ifdef __cplusplus
} ;
#endif

#endif
