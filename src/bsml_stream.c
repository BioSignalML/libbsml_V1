#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>

#include <libwebsockets.h>
#include <mhash.h>

#include "bsml_stream.h"
#include "bsml_internal.h"
#include "utility/bsml_string.h"
#include "utility/bsml_queue.h"
#include "utility/uri_parse.h"


#define min(a, b) ((a) < (b) ? (a) : (b))

#define BSML_STREAM_BLOCK_QUEUE_SIZE 2000  // Needs to depend on block size...??
                                           // B 500 (4000 bytes) --> Q 2000 ??

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


#define CHECKSUM_LENGTH  20    // SHA1 digest

struct bsml_StreamReader {
  BSML_STREAM_CHECKSUM checksum ;
  BSML_STREAM_READER_STATE state ;
  BSML_STREAM_ERROR_CODE error ;
  bsml_streamblock *block ;
  int number ;
  int version ;
  int expected ;
  char *storepos ;
  char *jsonhdr ;
	MHASH sha1 ;
  char checktext[2*CHECKSUM_LENGTH+1] ;
  } ;


const char *bsml_stream_double_type ;
const char *bsml_stream_long_type ;

static struct libwebsocket_context *context = NULL ;

static int bsml_stream_callback(struct libwebsocket_context *this, struct libwebsocket *ws,
                           enum libwebsocket_callback_reasons rsn, void *userdata, void *data, size_t len) ;

static void send_data_request(bsml_streamdata *stream, BSML_STREAM_CHECKSUM check) ;


static struct libwebsocket_protocols PROTOCOLS[] = {
    { BSML_STREAM_PROTOCOL, bsml_stream_callback, 0, 0 }, // Defaults packet size to 4096
    { NULL,                 NULL,                 0, 0 }
  } ;


void bsml_stream_initialise(void)
/*=============================*/
{
  if (context == NULL) {
    struct lws_context_creation_info *info = ALLOCATE(struct lws_context_creation_info) ;
    info->protocols = PROTOCOLS ;
    info->uid = -1 ;
    info->gid = -1 ;
    context = libwebsocket_create_context(info) ;
    free(info) ;
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


bsml_streamblock *bsml_streamblock_alloc(void)
/*============================================*/
{
  bsml_streamblock *blk = ALLOCATE(bsml_streamblock) ;
  if (blk) blk->number = -1 ;
  return blk ;
  }

void bsml_streamblock_free(bsml_streamblock *blk)
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

static void bsml_streamreader_free(bsml_streamreader *sp)
/*=====================================================*/
{
  if (sp) {
    bsml_streamblock_free(sp->block) ;
    if (sp->jsonhdr) free(sp->jsonhdr) ;
    free(sp) ;
    }
  }

static int bsml_stream_process_data(bsml_streamreader *sp, char *data, int len)
/*===========================================================================*/
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
          sp->sha1 = mhash_init(MHASH_SHA1) ;
          mhash(sp->sha1, "#", 1) ;
          if (sp->jsonhdr) free(sp->jsonhdr) ;
          sp->block = bsml_streamblock_alloc() ;
          sp->state = BSML_STREAM_STATE_TYPE ;
          }
        else len = 0 ;
        break ;
        }

      case BSML_STREAM_STATE_TYPE: {              // Getting block type
        sp->block->type = *pos ;
        pos++, len-- ;
        if (sp->block->type != '#') {
          mhash(sp->sha1, &sp->block->type, 1) ;
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
          mhash(sp->sha1, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) {
          if (*pos != 'V')
            sp->error = BSML_STREAM_ERROR_BAD_FORMAT ;
          else if (sp->version != BSML_STREAM_VERSION)
            sp->error = BSML_STREAM_ERROR_VERSION_MISMATCH ;
          else {
            mhash(sp->sha1, pos, 1) ;
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
          mhash(sp->sha1, pos, 1) ;
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
          mhash(sp->sha1, pos, delta) ;
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
          mhash(sp->sha1, pos, 1) ;
          pos += 1 ;
          len -= 1 ;
          }
        if (len > 0) sp->state = BSML_STREAM_STATE_HDREND ;
        break ;
        }

      case BSML_STREAM_STATE_HDREND: {            // Checking header LF
        if (*pos == '\n') {
          mhash(sp->sha1, pos, 1) ;
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
          mhash(sp->sha1, pos, delta) ;
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
          mhash(sp->sha1, pos, 1) ;
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
          sp->expected = 2*CHECKSUM_LENGTH ;
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
          unsigned char digest[CHECKSUM_LENGTH] ;
          mhash_deinit(sp->sha1, digest) ;
          char hexdigest[2*CHECKSUM_LENGTH + 1] ;
          int i ;
          for (i = 0 ;  i < CHECKSUM_LENGTH ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
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
      bsml_streamblock_free(sp->block) ;
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
  bsml_streamdata *sd = (bsml_streamdata *)userdata ;
  switch (rsn) {
   case LWS_CALLBACK_CLOSED:
   case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
    bsml_streamreader_free(sd->sp) ;
    sd->sp = NULL ;
    if (rsn == LWS_CALLBACK_CLIENT_CONNECTION_ERROR) {
      bsml_log_error("Client connection error\n") ;
      sd->error = BSML_STREAM_ERROR_NO_CONNECTION ;
      }
    sd->state = BSML_STREAM_CLOSED ;
    break ;

   case LWS_CALLBACK_CLIENT_ESTABLISHED:
    sd->sp = ALLOCATE(bsml_streamreader) ;
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
      return(-1) ;     // Will close and free session
      }
    break ;

   case LWS_CALLBACK_CLIENT_RECEIVE:
    while (sd->state != BSML_STREAM_ERROR && len > 0) {
      int n = bsml_stream_process_data(sd->sp, data, len) ;
      if (sd->sp->error == BSML_STREAM_ERROR_NONE) {
        if (sd->sp->state == BSML_STREAM_STATE_BLOCK) {
          if (bsml_queue_put(sd->blockQ, sd->sp->block) == 0) {
            bsml_log_error("Receive queue overflow...\n") ;
            bsml_log_error("Shutting down.\n") ;
            // But need to put NULL block to stop listener...
            sd->state = BSML_STREAM_CLOSED ;
            return(-1) ;     // Will close and free session
            }
          sd->sp->block = NULL ;
          sd->sp->state = BSML_STREAM_STATE_RESET ;
          if (bsml_queue_nearly_full(sd->blockQ)) {
// Doesn't stop flow (because server doesn't support flow control ???)
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


static void send_data_request(bsml_streamdata *sd, BSML_STREAM_CHECKSUM check)
/*==========================================================================*/
{
  char *size ;
  if (sd->maxsize > 0) asprintf(&size, ", \"maxsize\": %d", sd->maxsize) ;
  else                 asprintf(&size, "") ;
  char *hdr ;
  int n = asprintf(&hdr,
    "{\"uri\": \"%s\", \"start\": %f, \"duration\": %f, \"ctype\": \"%s\", \"dtype\": \"%s\"%s}",
    sd->uri, sd->start, sd->duration, bsml_stream_double_type, sd->dtype, size) ;
  free(size) ;
// Could have list of uris... [ "u1", "u2", ... ]
// And only send attributes if > 0....
// And send units if specified...

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 256 + LWS_SEND_BUFFER_POST_PADDING] ;
  char *bufp = (char *)buf + LWS_SEND_BUFFER_PRE_PADDING ;
  int buflen = sprintf(bufp, "#%c%dV%d%s0\n##", BSML_STREAM_DATA_REQUEST, BSML_STREAM_VERSION, n, hdr) ;
  free(hdr) ;

  if (check == BSML_STREAM_CHECKSUM_NONE) bufp += buflen ;
  else {
    MHASH sha1 = mhash_init(MHASH_SHA1) ;
    mhash(sha1, bufp, buflen) ;
    unsigned char digest[16] ;
    mhash_deinit(sha1, digest) ;
    bufp += buflen ;
    int i ;
    for (i = 0 ;  i < 16 ;  ++i) sprintf(bufp + 2*i, "%02x", digest[i]) ;
    buflen += 32 ;
    }
  *bufp = '\n' ;
  buflen += 1 ;
  libwebsocket_write(sd->ws, buf + LWS_SEND_BUFFER_PRE_PADDING, buflen, LWS_WRITE_BINARY) ;
  }


static void *run_websocket(void *arg)
/*=================================*/
{
  bsml_streamdata *sd = (bsml_streamdata *)arg ;
  parsed_uri *u = get_parsed_uri(sd->uri) ;
  if (u == NULL) return NULL ;
  int ssl = (strncmp(u->scheme, "https", 5) == 0) ? 1 : 0 ;
  const char *host = bsml_string_copy_len(u->host, u->host_len) ;
// If we are clever could we change WS rx_buffer_size based on sd->maxsize and sd->dtype...
// Or change it in the protocol structure ??
  sd->ws = libwebsocket_client_connect_extended(context, host, u->port, ssl, sd->uri,
                                                host, host, BSML_STREAM_PROTOCOL, -1, sd) ;
  while (sd->ws && sd->state != BSML_STREAM_ERROR && sd->state < BSML_STREAM_CLOSED) {
    libwebsocket_service(context, 1000) ;
    if (sd->state < BSML_STREAM_CLOSED && sd->stopped && !bsml_queue_nearly_full(sd->blockQ)) {
      sd->stopped = 0 ;
      libwebsocket_rx_flow_control(sd->ws, 1) ;
      }
    }
  bsml_queue_put(sd->blockQ, NULL) ;
  bsml_string_free(host) ;
  parsed_uri_free(u) ;
  pthread_exit(NULL) ;
  }


bsml_streamdata *bsml_streamdata_request(const char *uri, double start, double duration,
/*====================================================================================*/
                                                          const char *dtype, int maxsize)
{
  bsml_streamdata *sd = ALLOCATE(bsml_streamdata) ;
  if (sd) {
    sd->uri = bsml_string_copy(uri) ;
    sd->start = start ;
    sd->duration = duration ;
    sd->dtype = bsml_string_copy(dtype) ;
    sd->maxsize = maxsize ;
    sd->state = BSML_STREAM_STARTING ;
    sd->blockQ = bsml_queue_alloc(BSML_STREAM_BLOCK_QUEUE_SIZE) ;

    pthread_attr_t attr ;
    pthread_attr_init(&attr) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) ;
    if (pthread_create(&sd->thread, &attr, run_websocket, (void *)sd)) {
      bsml_log_error("Couldn't create thread for WebSocket\n") ;
      sd->thread = NULL ;
      bsml_streamdata_free(sd) ;
      sd = NULL ;
      }
    pthread_attr_destroy(&attr) ;
    }
  return sd ;
  }


bsml_streamblock *bsml_streamdata_read(bsml_streamdata *sd)
/*========================================================*/
{
  while (bsml_queue_empty(sd->blockQ))
    usleep(1000) ;
//bsml_log_error("Getting %d\n", bsml_queue_count(sd->blockQ)) ;
  return bsml_queue_get(sd->blockQ) ;
  }


void bsml_streamdata_free(bsml_streamdata *sd)
/*==========================================*/
{
  if (sd) {
    if (sd->thread) {
      pthread_cancel(sd->thread) ;
      pthread_join(sd->thread, NULL) ;
      }
    bsml_string_free(sd->uri) ;
    bsml_string_free(sd->dtype) ;
    bsml_queue_free(sd->blockQ) ;
    free(sd) ;
    }
  }
