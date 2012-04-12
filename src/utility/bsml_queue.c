/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdlib.h>

#include "bsml_internal.h"


typedef struct bsml_queue {
  int size ;
  int count ;
  void **head ;
  void **tail ;
  void **bufend ;
  void **buffer ;
  } bsml_queue ;


bsml_queue *bsml_queue_alloc(int size)
/*==================================*/
{
  bsml_queue *q = ALLOCATE(bsml_queue) ;
  if (q) {
    q->size = size ;
    q->buffer = calloc(sizeof(void *), size) ;
    q->head = q->tail = q->bufend ;
    }
  return q ;
  }

void bsml_queue_free(bsml_queue *q)
/*===============================*/
{
  if (q) {
    free(q->buffer) ;
    free(q) ;
    }
  }

int bsml_queue_put(bsml_queue *q, void *e)
/*======================================*/
{
  if (q->count < q->size) {
    *q->head = e ;
    ++q->count ;
    ++q->head ;
    if (q->head >= q->bufend) q->head = q->buffer ;
    return 1 ;
    }
  return 0 ;
  }

void *bsml_queue_get(bsml_queue *q)
/*===============================*/
{
  if (q->count > 0) {
    void *e = *q->tail ;
    --q->count ;
    ++q->tail ;
    if (q->tail >= q->bufend) q->tail = q->buffer ;
    return e ;
    }
  return NULL ;
  }

int bsml_queue_empty(bsml_queue *q)
/*===============================*/
{
  return (q->count <= 0) ;
  }

int bsml_queue_nearly_full(bsml_queue *q)
/*=====================================*/
{
  return (q->count >= 9*q->size/10) ;  // 90% full
  }

int bsml_queue_full(bsml_queue *q)
/*==============================*/
{
  return (q->count >= q->size) ;
  }

