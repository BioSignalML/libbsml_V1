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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>


#include "ssf.h"

#define min(a, b) ((a) < (b) ? (a) : (b))


char *stream_error_text(int errno)
/*==============================*/
{
  return ( (errno == STREAM_ERROR_UNEXPECTED_TRAILER) ? "Unexpected block trailer"
         : (errno == STREAM_ERROR_MISSING_HEADER_LF)  ? "Missing LF on header"
         : (errno == STREAM_ERROR_MISSING_TRAILER)    ? "Missing block trailer"
         : (errno == STREAM_ERROR_INVALID_CHECKSUM)   ? "Invalid block checksum"
         : (errno == STREAM_ERROR_MISSING_TRAILER_LF) ? "Missing LF on trailer"
         :                                              "" ) ;
  }


StreamReader *stream_new_reader(int file, int check)
/*================================================*/
{
  StreamReader *sp = (StreamReader *)calloc(1, sizeof(StreamReader)) ;
  if (sp) {
    sp->file = file ;
    sp->checksum = check ;
    sp->error = 0 ;
    sp->state = 0 ;
    sp->blockno = -1 ;
    sp->datalen = 0 ;
    sp->databuf = (char *)calloc(STREAM_BUFFER_SIZE, 1) ;
    if (sp->databuf == NULL) {
      free(sp) ;
      sp = NULL ;
      }
    sp->datapos = sp->databuf ;
    sp->jsonhdr = NULL ;
    sp->header = NULL ;
    sp->content = NULL ;
    sp->length = 0 ;
    }
  return sp ;
  }


static void stream_free_buffers(StreamReader *sp)
/*=============================================*/
{
  if (sp) {
    if (sp->jsonhdr) free(sp->jsonhdr) ;
    if (sp->header) cJSON_Delete(sp->header) ;
    if (sp->content) free(sp->content) ;
    sp->jsonhdr = sp->content = NULL ;
    sp->header = NULL ;
    }
  }


void stream_free_reader(StreamReader *sp)
/*=====================================*/
{
  if (sp) {
    stream_free_buffers(sp) ;
    if (sp->databuf) free(sp->databuf) ;
    free(sp) ;
    }
  }


int stream_read_block(StreamReader *sp)
/*===================================*/
{
  sp->error = 0 ;
  while (!sp->error && sp->state < 10) {

    if (sp->datalen <= 0) {
      sp->datalen = read(sp->file, sp->databuf, STREAM_BUFFER_SIZE) ;
      sp->datapos = sp->databuf ;
      if (sp->datalen <= 0) return 0 ;
      }

    switch (sp->state) {
      case 0: {            // Looking for a block
        char *next = memchr(sp->datapos, '#', sp->datalen) ;
        if (next) {
          sp->datalen -= (next - sp->datapos + 1) ;
          sp->datapos = next + 1 ;
          md5_init(&sp->md5) ;
          md5_append(&sp->md5, (const md5_byte_t *)"#", 1) ;
          stream_free_buffers(sp) ;
          sp->state = 1 ;
          }
        else sp->datalen = 0 ;
        break ;
        }

      case 1: {            // Getting block type 
        sp->type = *sp->datapos ;
        sp->datapos += 1 ;
        sp->datalen -= 1 ;
        if (sp->type != '#') {
          md5_append(&sp->md5, (const md5_byte_t *)&sp->type, 1) ;
          sp->blockno += 1 ;
          sp->expected = 0 ;
          sp->state = 2 ;
          }
        else sp->error = STREAM_ERROR_UNEXPECTED_TRAILER ;
        break ;
        }

      case 2: {            // Getting header length
        while (sp->datalen > 0 && isdigit(*sp->datapos)) {
          sp->expected = 10*sp->expected + (*sp->datapos - '0') ;
          md5_append(&sp->md5, (const md5_byte_t *)sp->datapos, 1) ;
          sp->datapos += 1 ;
          sp->datalen -= 1 ;
          }
        if (sp->datalen > 0) {
          sp->jsonhdr = calloc(sp->expected + 1, 1) ;
          sp->state = 3 ;
          }
        break ;
        }

      case 3: {            // Getting header JSON
        while (sp->datalen > 0 && sp->expected > 0) {
          int delta = min(sp->expected, sp->datalen) ;
          strncat(sp->jsonhdr, sp->datapos, delta) ;
          md5_append(&sp->md5, (const md5_byte_t *)sp->datapos, delta) ;
          sp->datapos += delta ;
          sp->datalen -= delta ;
          sp->expected -= delta ;
          }
        if (sp->expected == 0) {
          if (*sp->jsonhdr) sp->header = cJSON_Parse(sp->jsonhdr) ;
          sp->state = 4 ;
          }
        break ;
        }

      case 4: {            // Checking header LF
        if (*sp->datapos == '\n') {
          sp->datapos += 1 ;
          sp->datalen -= 1 ;
          md5_append(&sp->md5, (const md5_byte_t *)"\n", 1) ;
          sp->length = 0 ;
          if (sp->header) {
            cJSON *lenp = cJSON_GetObjectItem(sp->header, "length") ;
            if (lenp && lenp->type == cJSON_Number) sp->length = lenp->valueint ;
            cJSON_DeleteItemFromObject(sp->header, "length") ;
            }
          sp->content = calloc(sp->length+1, 1) ;
          sp->storepos = sp->content ;
          sp->expected = sp->length ;
          sp->state = 5 ;
          }
        else sp->error = STREAM_ERROR_MISSING_HEADER_LF ;
        break ;
        }

      case 5: {            // Getting content
        while (sp->datalen > 0 && sp->expected > 0) {
          int delta = min(sp->expected, sp->datalen) ;
          memcpy(sp->storepos, sp->datapos, delta) ;
          md5_append(&sp->md5, (const md5_byte_t *)sp->datapos, delta) ;
          sp->storepos += delta ;
          sp->datapos += delta ;
          sp->datalen -= delta ;
          sp->expected -= delta ;
          }
        if (sp->expected == 0) {
          sp->expected = 2 ;
          sp->state = 6 ;
          }
        break ;
        }

      case 6: {            // Getting trailer
        if (*sp->datapos == '#') {
          sp->datapos += 1 ;
          sp->datalen -= 1 ;
          sp->expected -= 1 ;
          if (sp->expected == 0) sp->state = 7 ;
          }
        else sp->error = STREAM_ERROR_MISSING_TRAILER ;
        break ;
        }

      case 7: {            // Checking for checksum
        if (*sp->datapos != '\n' && sp->checksum != STREAM_CHECKSUM_NONE) {
          sp->storepos = sp->checktext ;
          sp->expected = 32 ;
          sp->state = 8 ;
          }
        else sp->state = 9 ;
        sp->checktext[0] = '\0' ;
        break ;
        }

      case 8: {            // Getting checksum
        while (sp->datalen > 0 && sp->expected > 0 && isxdigit(*sp->datapos)) {
          *sp->storepos++ = *sp->datapos++ ;
          sp->datalen -= 1 ;
          sp->expected -= 1 ;
          }
        if (sp->datalen > 0) sp->state = 9 ;
        break ;
        }

      case 9: {            // Checking for final LF
        if (sp->checksum == STREAM_CHECKSUM_STRICT
         || sp->checksum == STREAM_CHECKSUM_CHECK && sp->checktext[0]) {
          md5_byte_t digest[16] ;
          char hexdigest[33] ;
          int i ;
          md5_finish(&sp->md5, digest) ;
          for (i = 0 ;  i < 16 ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
          if (strcmp(sp->checktext, hexdigest) != 0) sp->error = STREAM_ERROR_INVALID_CHECKSUM ;
          }
        if (sp->error == 0) {
          if (*sp->datapos == '\n') {
            sp->datapos += 1 ;
            sp->datalen -= 1 ;
            }
          else sp->error = STREAM_ERROR_MISSING_TRAILER_LF ;
          }
        sp->state = 10 ;    // All done, exit loop
        break ;
        }
      }
    }
  sp->state = 0 ;
  return 1 ;
  }
