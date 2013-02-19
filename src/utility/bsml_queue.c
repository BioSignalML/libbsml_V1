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
#include <pthread.h>

#include "bsml_queue.h"
#include "bsml_internal.h"

struct bsml_queue {
  int size ;
  int count ;
  void **head ;
  void **tail ;
  void **bufend ;
  void **buffer ;
  pthread_mutex_t lock ;
  } ;


bsml_queue *bsml_queue_alloc(int size)
/*==================================*/
{
  bsml_queue *q = ALLOCATE(bsml_queue) ;
  if (q) {
    q->size = size ;
    q->buffer = calloc(sizeof(void *), size) ;
    q->head = q->tail = q->buffer ;
    q->bufend = q->buffer + size ;
    pthread_mutex_init(&q->lock, NULL) ;
    }
  return q ;
  }

void bsml_queue_free(bsml_queue *q)
/*===============================*/
{
  if (q) {
    pthread_mutex_destroy(&q->lock) ;
    free(q->buffer) ;
    free(q) ;
    }
  }

int bsml_queue_put(bsml_queue *q, void *e)
/*======================================*/
{
  pthread_mutex_lock(&q->lock) ;
  if (q->count < q->size) {
    *q->head = e ;
    ++q->count ;
    ++q->head ;
    if (q->head >= q->bufend) q->head = q->buffer ;
    }
  int space = q->size - q->count ;
  pthread_mutex_unlock(&q->lock) ;
  return space ;
  }

void *bsml_queue_get(bsml_queue *q)
/*===============================*/
{
  if (q->count > 0) {
    pthread_mutex_lock(&q->lock) ;
    void *e = *q->tail ;
    --q->count ;
    ++q->tail ;
    if (q->tail >= q->bufend) q->tail = q->buffer ;
    pthread_mutex_unlock(&q->lock) ;
    return e ;
    }
  return NULL ;
  }


int bsml_queue_count(bsml_queue *q)
/*===============================*/
{
  return q->count ;
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

