#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <libwebsockets.h>


#include "md5/md5.h"
#include "cJSON/cJSON.h"



#define STREAM_PROTOCOL "biosignalml-ssf"


typedef enum {
  STREAM_ERROR_NONE = 0,
  STREAM_ERROR_UNEXPECTED_TRAILER,
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
  } StreamBlock ;


typedef struct {

  STREAM_CHECKSUM checksum ;
  STREAM_STATE state ;
  STREAM_ERROR error ;

  StreamBlock *block ;
  int number ;

  int expected ;
  char *storepos ;

  char *jsonhdr ;

	md5_state_t md5 ;
  char checktext[33] ;

  } StreamReader ;



#define min(a, b) ((a) < (b) ? (a) : (b))


char *stream_error_text(STREAM_ERROR errno)
/*=======================================*/
{
  return ( (errno == STREAM_ERROR_UNEXPECTED_TRAILER) ? "Unexpected block trailer"
         : (errno == STREAM_ERROR_MISSING_HEADER_LF)  ? "Missing LF on header"
         : (errno == STREAM_ERROR_MISSING_TRAILER)    ? "Missing block trailer"
         : (errno == STREAM_ERROR_INVALID_CHECKSUM)   ? "Invalid block checksum"
         : (errno == STREAM_ERROR_MISSING_TRAILER_LF) ? "Missing LF on trailer"
         : (errno == STREAM_ERROR_HASHRESERVED)       ? "Block type of '#' is reserved"
         : (errno == STREAM_ERROR_WRITEOF)            ? "Unexpected error when writing"
         :                                              "" ) ;
  }



StreamBlock *stream_new_block(void)
/*===============================*/
{
  StreamBlock *blk = (StreamBlock *)calloc(1, sizeof(StreamBlock)) ;
  if (blk) blk->number = -1 ;
  return blk ;
  }

void stream_free_block(StreamBlock *blk)
/*====================================*/
{
  if (blk) {
    if (blk->header) cJSON_Delete(blk->header) ;
    if (blk->content) free(blk->content) ;
    blk->header = NULL ;
    blk->content = NULL ;
    free(blk) ;
    }
  }




int stream_process_data(StreamReader *sp, char *data, int len)
//============================================================
{
  char *pos = data ;
  int size = len ;

  sp->error = STREAM_ERROR_NONE ;
  while (len > 0
      && sp->error == STREAM_ERROR_NONE
      && sp->state != STREAM_STATE_BLOCK) {

    switch (sp->state) {
      case STREAM_STATE_RESET: {             // Looking for a block
        char *next = memchr(pos, '#', len) ;
        if (next) {
          len -= (next - pos + 1) ;
          pos = next + 1 ;
          md5_init(&sp->md5) ;
          md5_append(&sp->md5, (const md5_byte_t *)"#", 1) ;

          if (sp->jsonhdr) free(sp->jsonhdr) ;
          sp->block = stream_new_block() ;

          sp->state = STREAM_STATE_TYPE ;
          }
        else len = 0 ;
        break ;
        }

      case STREAM_STATE_TYPE: {              // Getting block type
        sp->block->type = *pos ;
        pos++, len-- ;
        if (sp->block->type != '#') {
          md5_append(&sp->md5, (const md5_byte_t *)&sp->block->type, 1) ;
          sp->block->number = sp->number += 1 ;
          sp->expected = 0 ;
          sp->state = STREAM_STATE_HDRLEN ;
          }
        else sp->error = STREAM_ERROR_UNEXPECTED_TRAILER ;
        break ;
        }

      case STREAM_STATE_HDRLEN: {            // Getting header length
        while (len > 0 && isdigit(*pos)) {
          sp->expected = 10*sp->expected + (*pos - '0') ;
          md5_append(&sp->md5, (const md5_byte_t *)pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) {
          sp->jsonhdr = calloc(sp->expected + 1, 1) ;
          sp->state = STREAM_STATE_HEADER ;
          }
        break ;
        }

      case STREAM_STATE_HEADER: {            // Getting header JSON
        while (len > 0 && sp->expected > 0) {
          int delta = min(sp->expected, len) ;
          strncat(sp->jsonhdr, pos, delta) ;
          md5_append(&sp->md5, (const md5_byte_t *)pos, delta) ;
          pos += delta ;
          len -= delta ;
          sp->expected -= delta ;
          }
        if (sp->expected == 0) {
          if (*sp->jsonhdr) sp->block->header = cJSON_Parse(sp->jsonhdr) ;
          sp->state = STREAM_STATE_HDREND ;
          }
        break ;
        }

      case STREAM_STATE_HDREND: {            // Checking header LF
        if (*pos == '\n') {
          pos += 1 ;
          len -= 1 ;
          md5_append(&sp->md5, (const md5_byte_t *)"\n", 1) ;
          sp->block->length = 0 ;
          if (sp->block->header) {
            cJSON *lenp = cJSON_GetObjectItem(sp->block->header, "length") ;
            if (lenp && lenp->type == cJSON_Number) sp->block->length = lenp->valueint ;
            cJSON_DeleteItemFromObject(sp->block->header, "length") ;
            }
          sp->block->content = calloc(sp->block->length+1, 1) ;
          sp->storepos = sp->block->content ;
          sp->expected = sp->block->length ;
          sp->state = STREAM_STATE_CONTENT ;
          }
        else sp->error = STREAM_ERROR_MISSING_HEADER_LF ;
        break ;
        }

      case STREAM_STATE_CONTENT: {           // Getting content
        while (len > 0 && sp->expected > 0) {
          int delta = min(sp->expected, len) ;
          memcpy(sp->storepos, pos, delta) ;
          md5_append(&sp->md5, (const md5_byte_t *)pos, delta) ;
          sp->storepos += delta ;
          pos += delta ;
          len -= delta ;
          sp->expected -= delta ;
          }
        if (sp->expected == 0) {
          sp->expected = 2 ;
          sp->state = STREAM_STATE_TRAILER ;
          }
        break ;
        }

      case STREAM_STATE_TRAILER: {           // Getting trailer
        if (*pos == '#') {
          pos += 1 ;
          len -= 1 ;
          sp->expected -= 1 ;
          if (sp->expected == 0) sp->state = STREAM_STATE_CHECKSUM ;
          }
        else sp->error = STREAM_ERROR_MISSING_TRAILER ;
        break ;
        }

      case STREAM_STATE_CHECKSUM: {          // Checking for checksum
        if (*pos != '\n' && sp->checksum != STREAM_CHECKSUM_NONE) {
          sp->storepos = sp->checktext ;
          sp->expected = 32 ;
          sp->state = STREAM_STATE_CHECKDATA ;
          }
        else sp->state = STREAM_STATE_BLOCKEND ;
        sp->checktext[0] = '\0' ;
        break ;
        }

      case STREAM_STATE_CHECKDATA: {         // Getting checksum
        while (len > 0 && sp->expected > 0 && isxdigit(*pos)) {
          *sp->storepos++ = *pos++ ;
          len -= 1 ;
          sp->expected -= 1 ;
          }
        if (len > 0) sp->state = STREAM_STATE_BLOCKEND ;
        break ;
        }

      case STREAM_STATE_BLOCKEND: {          // Checking for final LF
        if (sp->checksum == STREAM_CHECKSUM_STRICT
         || sp->checksum == STREAM_CHECKSUM_CHECK && sp->checktext[0]) {
          md5_byte_t digest[16] ;
          char hexdigest[33] ;
          int i ;
          md5_finish(&sp->md5, digest) ;
          for (i = 0 ;  i < 16 ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
          if (strcmp(sp->checktext, hexdigest) != 0) sp->error = STREAM_ERROR_INVALID_CHECKSUM ;
          }
        if (sp->error == STREAM_ERROR_NONE) {
          if (*pos == '\n') {
            pos += 1 ;
            len -= 1 ;
            }
          else sp->error = STREAM_ERROR_MISSING_TRAILER_LF ;
          }
        sp->state = STREAM_STATE_BLOCK ;     // All done, exit loop
        break ;
        }
      }
    }

  if (sp->error) {
    if (sp->block) {
      stream_free_block(sp->block) ;
      sp->block = NULL ;
      }
    sp->state = STREAM_STATE_RESET ;
    }

  return(size - len) ;  // Bytes we've consumed
  }



void stream_process_block(StreamBlock *sb)
/*======================================*/
{
  printf("Got block %d: %c (%d)\n", sb->number, sb->type, sb->length, sb->header) ;

          if (sp->block->header) {
            cJSON *lenp = cJSON_GetObjectItem(sp->block->header, "length") ;
            if (lenp && lenp->type == cJSON_Number) sp->block->length = lenp->valueint ;
            cJSON_DeleteItemFromObject(sp->block->header, "length") ;

  }

/**
  * What we use to pass data to the callback...
  */

typedef struct {
  char *uri ;
  double start ;
  double end ;

  int state ;
  StreamReader *sp ;
  struct libwebsocket *ws ;
  } StreamData ;


static int stream_callback(struct libwebsocket_context *this,
/*=========================================================*/
                           struct libwebsocket *ws,
                           enum libwebsocket_callback_reasons rsn,
                           void *userdata, void *data, size_t len)
{
  StreamData *sd = (StreamData *)userdata ;

  switch (rsn) {
   case LWS_CALLBACK_CLOSED:
   case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    if (sd->sp) {
      if (sd->sp->state != STREAM_STATE_BLOCK && sd->sp->block) stream_free_block(sd->sp->block) ;
      if (sd->sp->jsonhdr) free(sd->sp->jsonhdr) ;
      free(sd->sp) ;
      }
    sd->state = 9 ;
    break ;

   case LWS_CALLBACK_CLIENT_ESTABLISHED:
    sd->sp = (StreamReader *)calloc(1, sizeof(StreamReader)) ;
    if (sd->sp) {
      sd->sp->state = STREAM_STATE_RESET ;
      sd->sp->number = -1 ;
      sd->state = 0 ;
      sd->ws = ws ;
      libwebsocket_callback_on_writable(this, ws) ;
      }
    else {             // Out of memory...
printf("No mem????\n") ;
      sd->state = 9 ;
      libwebsocket_close_and_free_session(this,	ws, LWS_CLOSE_STATUS_GOINGAWAY) ;
      }
    break ;

   case LWS_CALLBACK_CLIENT_RECEIVE:
    while (len > 0) {
//printf("Got %d bytes...\n", len) ;
      int n = stream_process_data(sd->sp, data, len) ;
      if (sd->sp->error == STREAM_ERROR_NONE) {
        if (sd->sp->state == STREAM_STATE_BLOCK) {
          // Take sd->sp->block and RESET state

          // But if can't take block then need to pause sender...
          libwebsocket_rx_flow_control(ws, 0) ;

          // And re-enable when OK...
          libwebsocket_rx_flow_control(ws, 1) ;


          stream_process_block(sd->sp->block) ;
          sd->sp->state = STREAM_STATE_RESET ;
          }
        }
      else {
        fprintf(stderr, "Stream block error: %s\n", stream_error_text(sd->sp->error)) ;
        }
      data += len ;
      len -= n ;
      }
    break ;

   case LWS_CALLBACK_CLIENT_WRITEABLE:
    if (sd->state == 0) {
      unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 256 + LWS_SEND_BUFFER_POST_PADDING] ;
      int n = sprintf(buf + LWS_SEND_BUFFER_PRE_PADDING,
                     "{'uri': '%s', 'start': %f, 'end': %f}", sd->uri, sd->start, sd->end) ;
      libwebsocket_write(ws, buf + LWS_SEND_BUFFER_PRE_PADDING, n, LWS_WRITE_TEXT) ;
      sd->state = 1 ;
      }
    break ;

   default:
    break ;
    }

  return 0 ;
  }


static struct libwebsocket_protocols protocols[] = {
    { STREAM_PROTOCOL, stream_callback, 0 },
    { NULL,            NULL,            0 }
  } ;


StreamData *stream_open(ctx, char *host, int port, char *uri, double start, double end)
/*==========*/
{
  struct libwebsocket *ws ;

  strm.uri = string_copy(uri) ;
  strm.start = start ;
  strm.end = end ;
  strm.state = -1 ;
  ws = libwebsocket_client_connect_extended(ctx, address, port, 0,
      "/stream/", address, address, STREAM_PROTOCOL, -1, &strm) ;




//  request from ws end point a stream of data

//  when data arrives call some callback function

  if (ws) while (libwebsocket_service(ctx, 10000) >= 0 && strm.state < 9) ;

  libwebsocket_context_destroy(ctx) ;    // Closes open sessions...
  }




int main(void)
/*==========*/
{
  int port = 8088 ;
  int use_ssl = 0 ;   // 2 ; // Allow self assigned
  const char *address = "localhost" ;

  struct libwebsocket_context *ctx ;
  struct libwebsocket *ws ;

  ctx = libwebsocket_create_context(CONTEXT_PORT_NO_LISTEN, NULL,
        protocols, libwebsocket_internal_extensions,  NULL, NULL, -1, -1, 0) ;

  if (ctx == NULL) {
    fprintf(stderr, "Creating libwebsocket context failed\n") ;
    return 1 ;
    }


  StreamData strm ;
  strm.state = -1 ;

  ws = libwebsocket_client_connect_extended(ctx, address, port, use_ssl,
      "/stream/", address, address, STREAM_PROTOCOL, -1, &strm) ;

//  request from ws end point a stream of data

//  when data arrives call some callback function

  if (ws) while (libwebsocket_service(ctx, 1) >= 0 && strm.state < 9) ;

  libwebsocket_context_destroy(ctx) ;    // Closes open sessions...
  }
