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

#ifdef __cplusplus
extern "C" {
#endif


#define STREAM_BUFFER_SIZE              4096

#define STREAM_ERROR_UNEXPECTED_TRAILER    1
#define STREAM_ERROR_MISSING_HEADER_LF     2
#define STREAM_ERROR_MISSING_TRAILER       3
#define STREAM_ERROR_INVALID_CHECKSUM      4
#define STREAM_ERROR_MISSING_TRAILER_LF    5

#define STREAM_CHECKSUM_STRICT             1
#define STREAM_CHECKSUM_CHECK              2
#define STREAM_CHECKSUM_IGNORE             3
#define STREAM_CHECKSUM_NONE               4


typedef struct {
  int error ;

  int blockno ;
  char type ;
  cJSON *header ;
  int length ;
  char *content ;

  char *contentp ;

  FILE *file ;
  int checksum ;

  int state ;
  int datalen ;
  char *databuf ;
  char *datapos ;
  int expected ;

  char *jsonhdr ;

	md5_state_t md5 ;
  char checktext[33] ;

  } StreamReader ;


char *stream_error_text(int) ;

StreamReader *stream_reader(FILE *, int) ;
void stream_free_buffers(StreamReader *) ;
void stream_free_reader(StreamReader *) ;

int stream_read_block(StreamReader *) ;


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
