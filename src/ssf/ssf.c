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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>


#include "ssf.h"

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

StreamBlock *stream_dup_block(StreamBlock *source)
/*==============================================*/
{
  StreamBlock *blk = (StreamBlock *)calloc(1, sizeof(StreamBlock)) ;
  if (blk) {
    if (source) memcpy(blk, source, sizeof(StreamBlock)) ;
    else        blk->number = -1 ;
    }
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
    }
  }


StreamReader *stream_new_input(STREAM_CHECKSUM check)
/*=================================================*/
{
  StreamReader *sp = (StreamReader *)calloc(1, sizeof(StreamReader)) ;
  if (sp) {
    sp->state = STREAM_STATE_RESET ;
    sp->file = -1 ;
    sp->checksum = check ;
    sp->block.number = -1 ;
    }
  return sp ;
  }


StreamReader *stream_new_reader(int file, STREAM_CHECKSUM check)
/*============================================================*/
{
  StreamReader *sp = stream_new_input(check) ;
  if (sp) {
    sp->file = file ;
    sp->databuf = (char *)calloc(STREAM_BUFFER_SIZE, 1) ;
    sp->datapos = sp->databuf ;
    if (sp->databuf == NULL) {
      free(sp) ;
      sp = NULL ;
      }
    }
  return sp ;
  }


static void stream_free_buffers(StreamReader *sp)
/*=============================================*/
{
  if (sp) {
    if (sp->block.header) cJSON_Delete(sp->block.header) ;
    if (sp->block.content) free(sp->block.content) ;
    if (sp->jsonhdr) free(sp->jsonhdr) ;
    sp->block.header = NULL ;
    sp->block.content = NULL ;
    sp->jsonhdr = NULL ;
    }
  }


void stream_free_input(StreamReader *sp)
/*====================================*/
{
  if (sp) {
    stream_free_buffers(sp) ;
    free(sp) ;
    }
  }


void stream_free_reader(StreamReader *sp)
/*=====================================*/
{
  if (sp) {
    if (sp->databuf) free(sp->databuf) ;
    stream_free_input(sp) ;
    }
  }


void stream_process_data(StreamReader *sp)
//========================================
{
  sp->block.error = 0 ;

  while (!sp->block.error && sp->state != STREAM_STATE_BLOCK) {

    if (sp->datalen <= 0) return ;   // Need more data

    switch (sp->state) {
      case STREAM_STATE_RESET: {            // Looking for a block
        char *next = memchr(sp->datapos, '#', sp->datalen) ;
        if (next) {
          sp->datalen -= (next - sp->datapos + 1) ;
          sp->datapos = next + 1 ;
          md5_init(&sp->md5) ;
          md5_append(&sp->md5, (const md5_byte_t *)"#", 1) ;
          stream_free_buffers(sp) ;
          sp->state = STREAM_STATE_TYPE ;
          }
        else sp->datalen = 0 ;
        break ;
        }

      case STREAM_STATE_TYPE: {            // Getting block type 
        sp->block.type = *sp->datapos ;
        sp->datapos += 1 ;
        sp->datalen -= 1 ;
        if (sp->block.type != '#') {
          md5_append(&sp->md5, (const md5_byte_t *)&sp->block.type, 1) ;
          sp->block.number += 1 ;
          sp->expected = 0 ;
          sp->state = STREAM_STATE_HDRLEN ;
          }
        else sp->block.error = STREAM_ERROR_UNEXPECTED_TRAILER ;
        break ;
        }

      case STREAM_STATE_HDRLEN: {            // Getting header length
        while (sp->datalen > 0 && isdigit(*sp->datapos)) {
          sp->expected = 10*sp->expected + (*sp->datapos - '0') ;
          md5_append(&sp->md5, (const md5_byte_t *)sp->datapos, 1) ;
          sp->datapos += 1 ;
          sp->datalen -= 1 ;
          }
        if (sp->datalen > 0) {
          sp->jsonhdr = calloc(sp->expected + 1, 1) ;
          sp->state = STREAM_STATE_HEADER ;
          }
        break ;
        }

      case STREAM_STATE_HEADER: {            // Getting header JSON
        while (sp->datalen > 0 && sp->expected > 0) {
          int delta = min(sp->expected, sp->datalen) ;
          strncat(sp->jsonhdr, sp->datapos, delta) ;
          md5_append(&sp->md5, (const md5_byte_t *)sp->datapos, delta) ;
          sp->datapos += delta ;
          sp->datalen -= delta ;
          sp->expected -= delta ;
          }
        if (sp->expected == 0) {
          if (*sp->jsonhdr) sp->block.header = cJSON_Parse(sp->jsonhdr) ;
          sp->state = STREAM_STATE_HDREND ;
          }
        break ;
        }

      case STREAM_STATE_HDREND: {            // Checking header LF
        if (*sp->datapos == '\n') {
          sp->datapos += 1 ;
          sp->datalen -= 1 ;
          md5_append(&sp->md5, (const md5_byte_t *)"\n", 1) ;
          sp->block.length = 0 ;
          if (sp->block.header) {
            cJSON *lenp = cJSON_GetObjectItem(sp->block.header, "length") ;
            if (lenp && lenp->type == cJSON_Number) sp->block.length = lenp->valueint ;
            cJSON_DeleteItemFromObject(sp->block.header, "length") ;
            }
          sp->block.content = calloc(sp->block.length+1, 1) ;
          sp->storepos = sp->block.content ;
          sp->expected = sp->block.length ;
          sp->state = STREAM_STATE_CONTENT ;
          }
        else sp->block.error = STREAM_ERROR_MISSING_HEADER_LF ;
        break ;
        }

      case STREAM_STATE_CONTENT: {            // Getting content
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
          sp->state = STREAM_STATE_TRAILER ;
          }
        break ;
        }

      case STREAM_STATE_TRAILER: {             // Getting trailer
        if (*sp->datapos == '#') {
          sp->datapos += 1 ;
          sp->datalen -= 1 ;
          sp->expected -= 1 ;
          if (sp->expected == 0) sp->state = STREAM_STATE_CHECKSUM ;
          }
        else sp->block.error = STREAM_ERROR_MISSING_TRAILER ;
        break ;
        }

      case STREAM_STATE_CHECKSUM: {            // Checking for checksum
        if (*sp->datapos != '\n' && sp->checksum != STREAM_CHECKSUM_NONE) {
          sp->storepos = sp->checktext ;
          sp->expected = 32 ;
          sp->state = STREAM_STATE_CHECKDATA ;
          }
        else sp->state = STREAM_STATE_BLOCKEND ;
        sp->checktext[0] = '\0' ;
        break ;
        }

      case STREAM_STATE_CHECKDATA: {           // Getting checksum
        while (sp->datalen > 0 && sp->expected > 0 && isxdigit(*sp->datapos)) {
          *sp->storepos++ = *sp->datapos++ ;
          sp->datalen -= 1 ;
          sp->expected -= 1 ;
          }
        if (sp->datalen > 0) sp->state = STREAM_STATE_BLOCKEND ;
        break ;
        }

      case STREAM_STATE_BLOCKEND: {            // Checking for final LF
        if (sp->checksum == STREAM_CHECKSUM_STRICT
         || sp->checksum == STREAM_CHECKSUM_CHECK && sp->checktext[0]) {
          md5_byte_t digest[16] ;
          char hexdigest[33] ;
          int i ;
          md5_finish(&sp->md5, digest) ;
          for (i = 0 ;  i < 16 ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
          if (strcmp(sp->checktext, hexdigest) != 0) sp->block.error = STREAM_ERROR_INVALID_CHECKSUM ;
          }
        if (sp->block.error == 0) {
          if (*sp->datapos == '\n') {
            sp->datapos += 1 ;
            sp->datalen -= 1 ;
            }
          else sp->block.error = STREAM_ERROR_MISSING_TRAILER_LF ;
          }
        sp->state = STREAM_STATE_BLOCK ;    // All done, exit loop
        break ;
        }
      }
    }
  }



StreamBlock *stream_read_block(StreamReader *sp)
/*============================================*/
{

  StreamBlock *result = NULL ;

  if (sp->datalen <= 0) {
    if (sp->file >= 0) sp->datalen = read(sp->file, sp->databuf, STREAM_BUFFER_SIZE) ;
    if (sp->datalen <= 0) {
      sp->state = STREAM_STATE_ENDED ;
      return NULL ;      // end of file...
      }
    sp->datapos = sp->databuf ;
    }

  stream_process_data(sp) ;  // returns needing data, or block error, or have good block

  if (sp->state == STREAM_STATE_BLOCK) {
    sp->state = STREAM_STATE_RESET ;
    result = stream_dup_block(&sp->block) ;
    sp->block.header = NULL ;  // Now pointed to be result->header
    sp->block.content = NULL ;
    }

  return result ;
  }



/* Stream writing... */


StreamWriter *stream_new_writer(int file, STREAM_CHECKSUM check)
/*============================================================*/
{
  StreamWriter *sp = (StreamWriter *)calloc(1, sizeof(StreamWriter)) ;
  if (sp) {
    sp->file = file ;
    sp->checksum = check ;
    }
  return sp ;
  }


void stream_free_writer(StreamWriter *sp)
/*=====================================*/
{
  if (sp) free(sp) ;
  }


static int stream_write_data(int fh, char *data, int len, md5_state_t *md5p)
/*========================================================================*/
{
  len = write(fh, data, len) ;
  if (len > -1 && md5p) md5_append(md5p, (const md5_byte_t *)data, len) ;
  return len ;
  }


int stream_write_block(StreamWriter *sp, StreamBlock *blk, STREAM_CHECKSUM check)
/*=============================================================================*/
{
  blk->error = 0 ;
  if (blk->type == '#') blk->error = STREAM_ERROR_HASHRESERVED ;
  else {
    int n ;
    char *json ;
    char lenbuf[32] ;

    md5_state_t md5, *md5p ;
    if (sp->checksum || check) {
      md5p = &md5 ;
      md5_init(md5p) ;
      }
    else md5p = NULL ;

    if (blk->header) {
      cJSON_DeleteItemFromObject(blk->header, "length") ;
      cJSON_AddNumberToObject(blk->header, "length", blk->length) ;
      json = cJSON_PrintUnformatted(blk->header) ;
      }
    else {
      json = calloc(32, 1) ;
      sprintf(json, "{\"length\":%d}", blk->length) ;
      }
    stream_write_data(sp->file, "#", 1, md5p) ;
    stream_write_data(sp->file, &blk->type, 1, md5p) ;
    sprintf(lenbuf, "%d", (int)strlen(json)) ;
    stream_write_data(sp->file, lenbuf, strlen(lenbuf), md5p) ;
    stream_write_data(sp->file, json, strlen(json), md5p) ;
    stream_write_data(sp->file, "\n", 1, md5p) ;
    free(json) ;
    n = 0 ;
    while (n < blk->length) {
      int w = min(blk->length - n, STREAM_BUFFER_SIZE) ;
      w = stream_write_data(sp->file, blk->content + n, w, md5p) ;
      if (w < 0) {
        blk->error = STREAM_ERROR_WRITEOF ;
        break ;
        }
      n += w ;
      }
    write(sp->file, "##", 2) ;
    if (md5p) {
      md5_byte_t digest[16] ;
      char hexdigest[33] ;
      int i ;
      md5_finish(md5p, digest) ;
      for (i = 0 ;  i < 16 ;  ++i) sprintf(hexdigest + 2*i, "%02x", digest[i]) ;
      write(sp->file, hexdigest, 32) ;
      }
    write(sp->file, "\n", 1) ;
    }
  return blk->error ;
  }
