#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <libwebsockets.h>
#include <mhash.h>

#include "bsml-internal.h"


static struct libwebsocket_context *context = NULL ;


static int stream_callback(struct libwebsocket_context *this, struct libwebsocket *ws,
                           enum libwebsocket_callback_reasons rsn, void *userdata, void *data, size_t len) ;


static struct libwebsocket_protocols protocols[] = {
    { STREAM_PROTOCOL, stream_callback, 0 },
    { NULL,            NULL,            0 }
  } ;



#define min(a, b) ((a) < (b) ? (a) : (b))


void stream_initialise(void)
/*========================*/
{
  if (context == NULL) {
    context = libwebsocket_create_context(CONTEXT_PORT_NO_LISTEN, NULL, protocols,
                                          libwebsocket_internal_extensions,  NULL, NULL, -1, -1, 0) ;
    if (context == NULL) {
      fprintf(stderr, "Creating libwebsocket context failed\n") ;
      exit(1) ;
      }
    }
  }

void stream_finish(void)
/*====================*/
{
  if (context) {
    libwebsocket_context_destroy(context) ;
    context = NULL ;
    }
  }


const char *stream_error_text(STREAM_ERROR_CODE code)
/*=================================================*/
{
  return (code == STREAM_ERROR_NONE)               ? ""
       : (code == STREAM_ERROR_UNEXPECTED_TRAILER) ? "Unexpected block trailer"
       : (code == STREAM_ERROR_MISSING_HEADER_LF)  ? "Missing LF on header"
       : (code == STREAM_ERROR_MISSING_TRAILER)    ? "Missing block trailer"
       : (code == STREAM_ERROR_INVALID_CHECKSUM)   ? "Invalid block checksum"
       : (code == STREAM_ERROR_MISSING_TRAILER_LF) ? "Missing LF on trailer"
       : (code == STREAM_ERROR_HASHRESERVED)       ? "Block type of '#' is reserved"
       : (code == STREAM_ERROR_WRITEOF)            ? "Unexpected error when writing"
       : (code == STREAM_ERROR_VERSION_MISMATCH)   ? "Block Stream has wring version"
       : (code == STREAM_ERROR_INVALID_MORE_FLAG)  ? "Invalid value for 'more' flag"
       : (code == STREAM_ERROR_BAD_JSON_HEADER)    ? "Incorrectly formatted JSON header"
       :                                             "Unknown Error" ;
  }



stream_block *stream_new_block(void)
/*================================*/
{
  stream_block *blk = (stream_block *)calloc(1, sizeof(stream_block)) ;
  if (blk) blk->number = -1 ;
  return blk ;
  }

void stream_free_block(stream_block *blk)
/*=====================================*/
{
  if (blk) {
    if (blk->header) json_decref(blk->header) ;
    if (blk->content) free(blk->content) ;
    blk->header = NULL ;
    blk->content = NULL ;
    free(blk) ;
    }
  }




int stream_process_data(stream_reader *sp, char *data, int len)
/*===========================================================*/
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
          sp->md5 = mhash_init(MHASH_MD5) ;
          mhash(sp->md5, "#", 1) ;

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
          mhash(sp->md5, &sp->block->type, 1) ;
          sp->block->number = sp->number += 1 ;
          sp->version = 0 ;
          sp->state = STREAM_STATE_VERSION ;
          }
        else sp->error = STREAM_ERROR_UNEXPECTED_TRAILER ;
        break ;
        }

      case STREAM_STATE_VERSION: {           // Getting version number
        while (len > 0 && isdigit(*pos)) {
          sp->version = 10*sp->version + (*pos - '0') ;
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) sp->state = STREAM_STATE_MORE ;
        break ;
        }

      case STREAM_STATE_MORE: {              // 'C' or 'M'
        if (sp->version != STREAM_VERSION) sp->error = STREAM_ERROR_VERSION_MISMATCH ;
        else if (*pos == 'C' || *pos == 'M') {
          sp->more = (*pos == 'M') ;
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          sp->expected = 0 ;
          sp->state = STREAM_STATE_HDRLEN ;
          }
        else sp->error = STREAM_ERROR_INVALID_MORE_FLAG ;
        break ;
        }

      case STREAM_STATE_HDRLEN: {            // Getting header length
        while (len > 0 && isdigit(*pos)) {
          sp->expected = 10*sp->expected + (*pos - '0') ;
          mhash(sp->md5, pos, 1) ;
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
          mhash(sp->md5, pos, delta) ;
          pos += delta ;
          len -= delta ;
          sp->expected -= delta ;
          }
        if (sp->expected == 0) {
          if (*sp->jsonhdr) sp->block->header = json_loads(sp->jsonhdr, 0, NULL) ;
          sp->state = STREAM_STATE_DATALEN ;
          }
        break ;
        }

      case STREAM_STATE_DATALEN: {           // Getting content length
        while (len > 0 && isdigit(*pos)) {
          sp->expected = 10*sp->expected + (*pos - '0') ;
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) sp->state = STREAM_STATE_HDREND ;
        break ;
        }

      case STREAM_STATE_HDREND: {            // Checking header LF
        if (*pos == '\n') {
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          sp->block->length = sp->expected ;
          sp->block->content = calloc(sp->expected+1, 1) ;
          sp->storepos = sp->block->content ;
          sp->state = STREAM_STATE_CONTENT ;
          }
        else sp->error = STREAM_ERROR_MISSING_HEADER_LF ;
        break ;
        }

      case STREAM_STATE_CONTENT: {           // Getting content
        while (len > 0 && sp->expected > 0) {
          int delta = min(sp->expected, len) ;
          memcpy(sp->storepos, pos, delta) ;
          mhash(sp->md5, pos, delta) ;
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
          mhash(sp->md5, pos, 1) ;
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
          unsigned char digest[16] ;
          mhash_deinit(sp->md5, digest) ;
          char hexdigest[33] ;
          int i ;
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



static int stream_callback(struct libwebsocket_context *this, struct libwebsocket *ws,
/*==================================================================================*/
                           enum libwebsocket_callback_reasons rsn, void *userdata, void *data, size_t len)
{
  stream_data *sd = (stream_data *)userdata ;

  switch (rsn) {
   case LWS_CALLBACK_CLOSED:
   case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    if (sd->sp) {
      if (sd->sp->state != STREAM_STATE_BLOCK && sd->sp->block) stream_free_block(sd->sp->block) ;
      if (sd->sp->jsonhdr) free(sd->sp->jsonhdr) ;
      free(sd->sp) ;
      }
    sd->state = STREAM_CLOSED ;
    break ;

   case LWS_CALLBACK_CLIENT_ESTABLISHED:
    sd->sp = (stream_reader *)calloc(1, sizeof(stream_reader)) ;
    if (sd->sp) {
      sd->sp->state = STREAM_STATE_RESET ;
      sd->sp->number = -1 ;
      sd->state = STREAM_OPENED ;
      sd->ws = ws ;
      libwebsocket_callback_on_writable(this, ws) ;
      }
    else {             // Out of memory...
      fprintf(stderr, "No memory for libwebsockets ????\n") ;
      sd->state = STREAM_CLOSED ;
      libwebsocket_close_and_free_session(this,	ws, LWS_CLOSE_STATUS_GOINGAWAY) ;
      }
    break ;

   case LWS_CALLBACK_CLIENT_RECEIVE:
    while (len > 0) {
//printf("Got %d bytes...\n", len) ;
      int n = stream_process_data(sd->sp, data, len) ;
      if (sd->sp->error == STREAM_ERROR_NONE) {

        if (sd->sp->state == STREAM_STATE_BLOCK) {
          if (sd->block == NULL) {
            sd->block = sd->sp->block ;
            sd->sp->block = NULL ;
            sd->sp->state = STREAM_STATE_RESET ;
            }
          else {
            libwebsocket_rx_flow_control(ws, 0) ;
            sd->stopped = 1 ;
            }
          }
        }

      else {
        fprintf(stderr, "Stream block error: %s\n", stream_error_text(sd->sp->error)) ;
        sd->error = sd->sp->error ;
        sd->state = STREAM_ERROR ;
        }
      data += len ;
      len -= n ;
      }
    break ;

   case LWS_CALLBACK_CLIENT_WRITEABLE:
    if (sd->state == STREAM_OPENED) {
      unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 256 + LWS_SEND_BUFFER_POST_PADDING] ;
      int n = sprintf(buf + LWS_SEND_BUFFER_PRE_PADDING,
                     "{'uri': '%s', 'start': %f, 'duration': %f}", sd->uri, sd->start, sd->duration) ;
      libwebsocket_write(ws, buf + LWS_SEND_BUFFER_PRE_PADDING, n, LWS_WRITE_TEXT) ;
      sd->state = STREAM_RUNNING ;
      }
    break ;

   default:
    break ;
    }

  return 0 ;
  }


stream_data *stream_new_data(const char *uri)
/*=========================================*/
{
  stream_data *sd = ALLOCATE(stream_data) ;
  if (sd) sd->uri = (unsigned char *)string_copy(uri) ;
  return sd ;
  }


stream_data *stream_data_request(const char *host, int port, const char *endpoint,
/*==============================================================================*/
                                 const char *uri, double start, double duration)
{
  stream_data *sd = stream_new_data(uri) ;
  if (sd) {
    sd->start = start ;
    sd->duration = duration ;
    sd->state = STREAM_STARTING ;
    sd->ws = libwebsocket_client_connect_extended(context, host, port, 0, endpoint, host, host,
                                                  STREAM_PROTOCOL, -1, sd) ;
    }
  return sd ;
  }


stream_block *stream_data_read(stream_data *sd)
/*===========================================*/
{
  stream_block *result = NULL ;

  while (sd->ws && sd->block == NULL && sd->state < STREAM_CLOSED)
    libwebsocket_service(context, 10000) ;

  if (sd->block != NULL) {
    result = sd->block ;
    sd->block = NULL ;
    if (sd->state < STREAM_CLOSED && sd->stopped) {
      sd->stopped = 0 ;
      libwebsocket_rx_flow_control(sd->ws, 1) ;
      libwebsocket_service(context, 0) ;
      }
    }
  return result ;
  }


void stream_free_data(stream_data *sd)
/*==================================*/
{
  if (sd) {
    if (sd->uri) free(sd->uri) ;
    free(sd) ;
    }
  }
