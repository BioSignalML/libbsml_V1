#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <libwebsockets.h>
#include <mhash.h>

#include "bsml_stream.h"
#include "bsml_internal.h"
#include "utility/bsml_string.h"
#include "utility/bsml_queue.h"


#define min(a, b) ((a) < (b) ? (a) : (b))

#define BSML_STREAM_BLOCK_QUEUE_SIZE 100


typedef enum {
  BSML_STREAM_STATE_ENDED = -1,
  BSML_STREAM_STATE_RESET,
  BSML_STREAM_STATE_TYPE,
  BSML_STREAM_STATE_VERSION,
  BSML_STREAM_STATE_HDRLEN,
  BSML_STREAM_STATE_HEADER,
  BSML_STREAM_STATE_DATALEN,
  BSML_STREAM_STATE_HDREND,
  BSML_STREAM_STATE_CONTENT,
  BSML_STREAM_STATE_TRAILER,
  BSML_STREAM_STATE_CHECKSUM,
  BSML_STREAM_STATE_CHECKDATA,
  BSML_STREAM_STATE_BLOCKEND,
  BSML_STREAM_STATE_BLOCK
  } BSML_STREAM_READER_STATE ;


struct bsml_Stream_Reader {
  BSML_STREAM_CHECKSUM checksum ;
  BSML_STREAM_READER_STATE state ;
  BSML_STREAM_ERROR_CODE error ;
  bsml_stream_block *block ;
  int number ;

  int version ;
  int expected ;
  char *storepos ;
  char *jsonhdr ;
	MHASH md5 ;
  char checktext[33] ;
  } ;


const char *bsml_stream_double_type ;
const char *bsml_stream_long_type ;

static struct libwebsocket_context *context = NULL ;

static int bsml_stream_callback(struct libwebsocket_context *this, struct libwebsocket *ws,
                           enum libwebsocket_callback_reasons rsn, void *userdata, void *data, size_t len) ;

static void send_data_request(bsml_stream_data *sd, BSML_STREAM_CHECKSUM check) ;


static struct libwebsocket_protocols protocols[] = {
    { BSML_STREAM_PROTOCOL, bsml_stream_callback, 0 },
    { NULL,            NULL,            0 }
  } ;


void bsml_stream_initialise(void)
/*=============================*/
{
  if (context == NULL) {
    context = libwebsocket_create_context(CONTEXT_PORT_NO_LISTEN, NULL, protocols,
                                          libwebsocket_internal_extensions,  NULL, NULL, -1, -1, 0) ;
    if (context == NULL) {
      bsml_log_error("Creating libwebsocket context failed\n") ;
      exit(1) ;
      }
    }
  short one = 0x0001 ;
  int little = *((char *)&one) == 1 ;
  char endiness = little ? '<' : '>' ;
  asprintf((char **)(&bsml_stream_double_type), "%cf%d", endiness, sizeof(double)) ;
  asprintf((char **)(&bsml_stream_long_type),   "%ci%d", endiness, sizeof(long)) ;
  }


void bsml_stream_finish(void)
/*=========================*/
{
  if (context) {
    libwebsocket_context_destroy(context) ;
    context = NULL ;
    }
  }


const char *bsml_stream_error_text(BSML_STREAM_ERROR_CODE code)
/*===========================================================*/
{
  return (code == BSML_STREAM_ERROR_NONE)               ? ""
       : (code == BSML_STREAM_ERROR_UNEXPECTED_TRAILER) ? "Unexpected block trailer"
       : (code == BSML_STREAM_ERROR_MISSING_HEADER_LF)  ? "Missing LF on header"
       : (code == BSML_STREAM_ERROR_MISSING_TRAILER)    ? "Missing block trailer"
       : (code == BSML_STREAM_ERROR_INVALID_CHECKSUM)   ? "Invalid block checksum"
       : (code == BSML_STREAM_ERROR_MISSING_TRAILER_LF) ? "Missing LF on trailer"
       : (code == BSML_STREAM_ERROR_HASHRESERVED)       ? "Block type of '#' is reserved"
       : (code == BSML_STREAM_ERROR_WRITEOF)            ? "Unexpected error when writing"
       : (code == BSML_STREAM_ERROR_VERSION_MISMATCH)   ? "Block Stream has wring version"
       : (code == BSML_STREAM_ERROR_BAD_JSON_HEADER)    ? "Incorrectly formatted JSON header"
       : (code == BSML_STREAM_ERROR_BAD_FORMAT)         ? "Incorrect message format"
       :                                                  "Unknown Error" ;
  }



bsml_stream_block *bsml_stream_block_alloc(void)
/*============================================*/
{
  bsml_stream_block *blk = ALLOCATE(bsml_stream_block) ;
  if (blk) blk->number = -1 ;
  return blk ;
  }

void bsml_stream_block_free(bsml_stream_block *blk)
/*===============================================*/
{
  if (blk) {
    if (blk->header) json_decref(blk->header) ;
    if (blk->content) free(blk->content) ;
    blk->header = NULL ;
    blk->content = NULL ;
    free(blk) ;
    }
  }



int bsml_stream_process_data(bsml_stream_reader *sp, char *data, int len)
/*=====================================================================*/
{
  char *pos = data ;
  int size = len ;

  sp->error = BSML_STREAM_ERROR_NONE ;
  while (len > 0
      && sp->error == BSML_STREAM_ERROR_NONE
      && sp->state != BSML_STREAM_STATE_BLOCK) {

    switch (sp->state) {
      case BSML_STREAM_STATE_RESET: {             // Looking for a block
        char *next = memchr(pos, '#', len) ;
        if (next) {
          len -= (next - pos + 1) ;
          pos = next + 1 ;
          sp->md5 = mhash_init(MHASH_MD5) ;
          mhash(sp->md5, "#", 1) ;
          if (sp->jsonhdr) free(sp->jsonhdr) ;
          sp->block = bsml_stream_block_alloc() ;
          sp->state = BSML_STREAM_STATE_TYPE ;
          }
        else len = 0 ;
        break ;
        }

      case BSML_STREAM_STATE_TYPE: {              // Getting block type
        sp->block->type = *pos ;
        pos++, len-- ;
        if (sp->block->type != '#') {
          mhash(sp->md5, &sp->block->type, 1) ;
          sp->block->number = sp->number += 1 ;
          sp->version = 0 ;
          sp->state = BSML_STREAM_STATE_VERSION ;
          }
        else sp->error = BSML_STREAM_ERROR_UNEXPECTED_TRAILER ;
        break ;
        }

      case BSML_STREAM_STATE_VERSION: {           // Getting version number
        while (len > 0 && isdigit(*pos)) {
          sp->version = 10*sp->version + (*pos - '0') ;
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) {
          if (*pos != 'V')
            sp->error = BSML_STREAM_ERROR_BAD_FORMAT ;
          else if (sp->version != BSML_STREAM_VERSION)
            sp->error = BSML_STREAM_ERROR_VERSION_MISMATCH ;
          else {
            pos += 1 ;
            len -= 1 ;
            sp->expected = 0 ;
            sp->state = BSML_STREAM_STATE_HDRLEN ;
            }
          }
        break ;
        }

      case BSML_STREAM_STATE_HDRLEN: {            // Getting header length
        while (len > 0 && isdigit(*pos)) {
          sp->expected = 10*sp->expected + (*pos - '0') ;
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) {
          sp->jsonhdr = calloc(sp->expected + 1, 1) ;
          sp->state = BSML_STREAM_STATE_HEADER ;
          }
        break ;
        }

      case BSML_STREAM_STATE_HEADER: {            // Getting header JSON
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
          sp->state = BSML_STREAM_STATE_DATALEN ;
          }
        break ;
        }

      case BSML_STREAM_STATE_DATALEN: {           // Getting content length
        while (len > 0 && isdigit(*pos)) {
          sp->expected = 10*sp->expected + (*pos - '0') ;
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) sp->state = BSML_STREAM_STATE_HDREND ;
        break ;
        }

      case BSML_STREAM_STATE_HDREND: {            // Checking header LF
        if (*pos == '\n') {
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          sp->block->length = sp->expected ;
          sp->block->content = calloc(sp->expected+1, 1) ;
          sp->storepos = sp->block->content ;
          sp->state = BSML_STREAM_STATE_CONTENT ;
          }
        else sp->error = BSML_STREAM_ERROR_MISSING_HEADER_LF ;
        break ;
        }

      case BSML_STREAM_STATE_CONTENT: {           // Getting content
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
          sp->state = BSML_STREAM_STATE_TRAILER ;
          }
        break ;
        }

      case BSML_STREAM_STATE_TRAILER: {           // Getting trailer
        if (*pos == '#') {
          mhash(sp->md5, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          sp->expected -= 1 ;
          if (sp->expected == 0) sp->state = BSML_STREAM_STATE_CHECKSUM ;
          }
        else sp->error = BSML_STREAM_ERROR_MISSING_TRAILER ;
        break ;
        }

      case BSML_STREAM_STATE_CHECKSUM: {          // Checking for checksum
        if (*pos != '\n' && sp->checksum != BSML_STREAM_CHECKSUM_NONE) {
          sp->storepos = sp->checktext ;
          sp->expected = 32 ;
          sp->state = BSML_STREAM_STATE_CHECKDATA ;
          }
        else sp->state = BSML_STREAM_STATE_BLOCKEND ;
        sp->checktext[0] = '\0' ;
        break ;
        }

      case BSML_STREAM_STATE_CHECKDATA: {         // Getting checksum
        while (len > 0 && sp->expected > 0 && isxdigit(*pos)) {
          *sp->storepos++ = *pos++ ;
          len -= 1 ;
          sp->expected -= 1 ;
          }
        if (len > 0) sp->state = BSML_STREAM_STATE_BLOCKEND ;
        break ;
        }

      case BSML_STREAM_STATE_BLOCKEND: {          // Checking for final LF
        if (sp->checksum == BSML_STREAM_CHECKSUM_STRICT
         || sp->checksum == BSML_STREAM_CHECKSUM_CHECK && sp->checktext[0]) {
          unsigned char digest[16] ;
          mhash_deinit(sp->md5, digest) ;
          char hexdigest[33] ;
          int i ;
          for (i = 0 ;  i < 16 ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
          if (strcmp(sp->checktext, hexdigest) != 0) sp->error = BSML_STREAM_ERROR_INVALID_CHECKSUM ;
          }
        if (sp->error == BSML_STREAM_ERROR_NONE) {
          if (*pos == '\n') {
            pos += 1 ;
            len -= 1 ;
            }
          else sp->error = BSML_STREAM_ERROR_MISSING_TRAILER_LF ;
          }
        sp->state = BSML_STREAM_STATE_BLOCK ;     // All done, exit loop
        break ;
        }
      }
    }

  if (sp->error) {
    if (sp->block) {
      bsml_stream_block_free(sp->block) ;
      sp->block = NULL ;
      }
    sp->state = BSML_STREAM_STATE_RESET ;
    }

  return(size - len) ;  // Bytes we've consumed
  }



static int bsml_stream_callback(struct libwebsocket_context *this, struct libwebsocket *ws,
/*==================================================================================*/
                           enum libwebsocket_callback_reasons rsn, void *userdata, void *data, size_t len)
{
  bsml_stream_data *sd = (bsml_stream_data *)userdata ;

  switch (rsn) {
   case LWS_CALLBACK_CLOSED:
   case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    if (sd->sp) {
      if (sd->sp->state != BSML_STREAM_STATE_BLOCK && sd->sp->block) bsml_stream_block_free(sd->sp->block) ;
      if (sd->sp->jsonhdr) free(sd->sp->jsonhdr) ;
      free(sd->sp) ;
      }
    if (rsn == LWS_CALLBACK_CLIENT_CONNECTION_ERROR)
      sd->error = BSML_STREAM_ERROR_NO_CONNECTION ;
    sd->state = BSML_STREAM_CLOSED ;
    break ;

   case LWS_CALLBACK_CLIENT_ESTABLISHED:
    sd->sp = ALLOCATE(bsml_stream_reader) ;
    if (sd->sp) {
      sd->sp->state = BSML_STREAM_STATE_RESET ;
      sd->sp->checksum = BSML_STREAM_CHECKSUM_CHECK ;  // Set from stream data structure ??
      sd->sp->number = -1 ;
      sd->state = BSML_STREAM_OPENED ;
      sd->ws = ws ;
      libwebsocket_callback_on_writable(this, ws) ;
      }
    else {             // Out of memory...
      bsml_log_error("No memory for libwebsockets ????\n") ;
      sd->state = BSML_STREAM_CLOSED ;
      libwebsocket_close_and_free_session(this,  ws, LWS_CLOSE_STATUS_GOINGAWAY) ;
      exit(1) ;
      }
    break ;

   case LWS_CALLBACK_CLIENT_RECEIVE:
    while (sd->state != BSML_STREAM_ERROR && len > 0) {
      int n = bsml_stream_process_data(sd->sp, data, len) ;
      if (sd->sp->error == BSML_STREAM_ERROR_NONE) {
        if (sd->sp->state == BSML_STREAM_STATE_BLOCK) {
          bsml_queue_put(sd->blockQ, sd->sp->block) ;
          sd->sp->block = NULL ;
          sd->sp->state = BSML_STREAM_STATE_RESET ;
          if (bsml_queue_nearly_full(sd->blockQ)) {
            libwebsocket_rx_flow_control(ws, 0) ;
            sd->stopped = 1 ;
            }
          }
        }
      else {
        sd->error = sd->sp->error ;
        sd->state = BSML_STREAM_ERROR ;
        }
      data += len ;
      len -= n ;
      }
    break ;

   case LWS_CALLBACK_CLIENT_WRITEABLE:
    if (sd->state == BSML_STREAM_OPENED) {
      send_data_request(sd, BSML_STREAM_CHECKSUM_STRICT) ;
      sd->state = BSML_STREAM_RUNNING ;
      }
    break ;

   default:
    break ;
    }

  return 0 ;
  }


static void send_data_request(bsml_stream_data *sd, BSML_STREAM_CHECKSUM check)
/*===========================================================================*/
{
  char *hdr ;
  int n = asprintf(&hdr,
    "{\"uri\": \"%s\", \"start\": %f, \"duration\": %f, \"ctype\": \"%s\", \"dtype\": \"%s\"}",
    sd->uri, sd->start, sd->duration, bsml_stream_double_type, sd->dtype) ;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 256 + LWS_SEND_BUFFER_POST_PADDING] ;
  char *bufp = (char *)buf + LWS_SEND_BUFFER_PRE_PADDING ;
  int buflen = sprintf(bufp, "#%c%d%c%d%s0\n##", BSML_STREAM_DATA_REQUEST, BSML_STREAM_VERSION, 'C', n, hdr) ;
  free(hdr) ;

  if (check == BSML_STREAM_CHECKSUM_NONE) bufp += buflen ;
  else {
    MHASH md5 = mhash_init(MHASH_MD5) ;
    mhash(md5, bufp, buflen) ;
    unsigned char digest[16] ;
    mhash_deinit(md5, digest) ;
    bufp += buflen ;
    int i ;
    for (i = 0 ;  i < 16 ;  ++i) sprintf(bufp + 2*i, "%02x", digest[i]) ;
    buflen += 32 ;
    }
  *bufp = '\n' ;
  buflen += 1 ;
  libwebsocket_write(sd->ws, buf + LWS_SEND_BUFFER_PRE_PADDING, buflen, LWS_WRITE_BINARY) ;
  }


bsml_stream_data *bsml_stream_data_alloc(const char *uri)
/*=====================================================*/
{
  bsml_stream_data *sd = ALLOCATE(bsml_stream_data) ;
  if (sd) sd->uri = bsml_string_copy(uri) ;
  return sd ;
  }


bsml_stream_data *bsml_stream_data_request(const char *host, int port, const char *endpoint,
/*========================================================================================*/
                                 const char *uri, double start, double duration, const char *dtype)
{
  bsml_stream_data *sd = bsml_stream_data_alloc(uri) ;
  if (sd) {
    sd->start = start ;
    sd->duration = duration ;
    sd->dtype = bsml_string_copy(dtype) ;
    sd->state = BSML_STREAM_STARTING ;
    sd->ws = libwebsocket_client_connect_extended(context, host, port, 0, endpoint, host, host,
                                                  BSML_STREAM_PROTOCOL, -1, sd) ;
    sd->blockQ = bsml_queue_alloc(BSML_STREAM_BLOCK_QUEUE_SIZE) ;
    }
  return sd ;
  }


bsml_stream_block *bsml_stream_data_read(bsml_stream_data *sd)
/*==========================================================*/
{
  bsml_stream_block *result = NULL ;
  while (sd->ws && bsml_queue_empty(sd->blockQ)
    && sd->state != BSML_STREAM_ERROR && sd->state < BSML_STREAM_CLOSED) {
    libwebsocket_service(context, 10) ;
    }
  if (!bsml_queue_empty(sd->blockQ)) {
    result = bsml_queue_get(sd->blockQ) ;
    if (sd->state < BSML_STREAM_CLOSED && sd->stopped) {
      sd->stopped = 0 ;
      libwebsocket_rx_flow_control(sd->ws, 1) ;
      libwebsocket_service(context, 0) ;
      }
    }
  return result ;
  }


void bsml_stream_data_free(bsml_stream_data *sd)
/*============================================*/
{
  if (sd) {
    bsml_string_free(sd->uri) ;
    bsml_string_free(sd->dtype) ;
    bsml_queue_free(sd->blockQ) ;
    free(sd) ;
    }
  }
