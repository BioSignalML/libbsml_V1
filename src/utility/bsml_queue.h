/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/
/** @file
  * FIFO queue.
  */


#ifndef _BSML_QUEUE_H
#define _BSML_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bsml_queue {
  int size ;
  int count ;
  void **head ;
  void **tail ;
  void **buffer ;
  } bsml_queue ;


bsml_queue *bsml_queue_alloc(int size) ;

void bsml_queue_free(bsml_queue *q) ;

int bsml_queue_put(bsml_queue *q, void *e) ;

void *bsml_queue_get(bsml_queue *q) ;

int bsml_queue_empty(bsml_queue *q) ;

int bsml_queue_nearly_full(bsml_queue *q) ;

int bsml_queue_full(bsml_queue *q) ;


#ifdef __cplusplus
  } ;
#endif

#endif
