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
#include <string.h>

#include "md5/md5.h"
#include "cJSON/cJSON.h"

#include "ssf.h"


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


StreamReader *stream_reader(FILE *file, int check)
/*==============================================*/
{
  StreamReader *sp = (StreamReader *)calloc(1, sizeof(StreamReader)) ;
  if (sp == NULL) raise_error("Out of memory...") ;
  else {
    sp->file = file ;
    sp->checksum = check ;
    sp->error = 0 ;
    sp->state = 0 ;
    sp->blockno = -1 ;
    sp->datalen = 0 ;
    sp->databuf = (char *)calloc(STREAM_BUFFER_SIZE, 1) ;
    if (sp->databuf == NULL) {
      raise_error("Out of memory...") ;
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


void stream_free_buffers(StreamReader *sp)
/*======================================*/
{
  if (sp) {
    if (sp->databuf) free(sp->databuf) ;
    if (sp->jsonhdr) free(sp->jsonhdr) ;
    if (sp->header) cJOSN_Delete(sp->header) ;
    if (sp->content) free(sp->header) ;
    sp->databuf = sp->jsonhdr = sp->content = NULL ;
    sp->jsonhdr = NULL ;
    }
  }


void stream_free_reader(StreamReader *sp)
/*=====================================*/
{
  if (sp) {
    stream_free_buffers(sp) ;
    free(sp) ;
    }
  }


int stream_read_block(StreamReader *sp)
/*===================================*/
{

  while (1) {

    if (sp->datalen <= 0) {
      sp->datalen = read(sp->file, STREAM_BUFFER_SIZE, sp->databuf) ;
      sp->datapos = sp->databuf ;
      if (sp->datalen <= 0) return 0 ;
      }

    if      (sp->state == 0) {            // Looking for a block
      char *next = memchr(sp->datapos, '#', sp->datalen) ;
      if (next) {
        sp->datalen -= (next - sp->datapos + 1) ;
        sp->datapos = next + 1 ;
        md5_init(&sp->md5) ;
        md5_append(&sp->md5, (const md5_byte_t *)"#", 1) ;
        stream_free_buffers(sp) ;
        sp->error = 0 ;
        sp->state = 1 ;
        }
      else {
        sp->datalen = 0 ;
        }
      }

    else if (sp->state == 1) {            // Getting block type 
      sp->type = *sp->datapos ;
      sp->datapos += 1 ;
      sp->datalen -= 1 ;
      if (sp->type != '#') {
        md5_append(&sp->md5, (const md5_byte_t *)&sp->type, 1) ;
        sp->blockno += 1 ;
        sp->expected = 0 ;
        sp->state = 2 ;
        }
      else {
        sp->error = STREAM_ERROR_UNEXPECTED_TRAILER ;
        sp->state = 0 ;
        }
      }

    else if (sp->state == 2) {            // Getting header length
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
      }

    else if (sp->state == 3) {            // Getting header JSON
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
      }

    else if (sp->state == 4) {            // Checking header LF
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
        sp->content = calloc(sp->length, 1) ;
        sp->contentp = sp->content ;
        sp->expected = sp->length ;
        sp->state = 5 ;
        }
      else {
        sp->error = STREAM_ERROR_MISSING_HEADER_LF ;
        sp->state = 0 ;
        }
      }

    else if (sp->state == 5) {            // Getting content
      while (sp->datalen > 0 && sp->expected > 0) {
        int delta = min(sp->expected, sp->datalen) ;
        memcpy(sp->contentp, sp->datapos, delta) ;
        md5_append(&sp->md5, (const md5_byte_t *)sp->datapos, delta) ;
        sp->contentp += delta ;
        sp->datapos += delta ;
        sp->datalen -= delta ;
        sp->expected -= delta ;
        }
      if (sp->expected == 0) {
        sp->expected = 2 ;
        sp->state = 6 ;
        }
      }

    else if (sp->state == 6) {            // Getting trailer
      if (*sp->datapos == '#') {
        sp->datapos += 1 ;
        sp->datalen -= 1 ;
        sp->expected -= 1 ;
        if (sp->expected == 0) sp->state = 7 ;
        }
      else {
        sp->error = STREAM_ERROR_MISSING_TRAILER ;
        sp->state = 0 ;
        }
      }

    else if (sp->state == 7) {            // Checking for checksum
      if (*sp->datapos != '\n' && sp->checksum != STREAM_CHECKSUM_NONE) {
        sp->expected = 32 ;
        sp->state = 8 ;
        }
      else sp->state = 9 ;
      sp->checktext[0] = '\0' ;
      }

    else if (sp->state == 8) {            // Getting checksum
      while (sp->datalen > 0 && sp->expected > 0) {
        int delta = min(sp->expected, sp->datalen) ;
        strncat(sp->checktext, sp->datapos, delta) ;
        sp->datapos += delta ;
        sp->datalen -= delta ;
        sp->expected -= delta ;
        }
      if (sp->expected == 0) sp->state = 9 ;
      }

    else if (sp->state == 9) {            // Checking for final LF
      if (sp->checksum == STREAM_CHECKSUM_STRICT
       || sp->checksum == STREAM_CHECKSUM_CHECK && sp->checktext[0]) {
        md5_byte_t digest[16] ;
        char hexdigest[33] ;
        int i ;
        md5_finish(&sp->md5, digest) ;
        for (i = 0 ;  i < 16 ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
        if (strcmp(sp->checktext, hexdigest) != 0) sp->error = STREAM_ERROR_INVALID_CHECKSUM ;
        }
      if (sp->error == 0 && *sp->datapos == '\n') {
        sp->datapos += 1 ;
        sp->datalen -= 1 ;
        return 1 ;
        }
      else {
        sp->error = STREAM_ERROR_MISSING_TRAILER_LF ;
        }
      sp->state = 0 ;
      }
    }
  }
