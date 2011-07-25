#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int buf_size ;
  int buf_count ;
  int buf_pos ;
  int min_list_lo ;
  int min_list_hi ;
  int min_list_count ;
  double min_total ;
  int max_list_lo ;
  int max_list_hi ;
  int max_list_count ;
  double max_total ;
  int *min_list ;
  int *max_list ;
  double *buffer ;
  } AutoScaler ;

AutoScaler *autoscaler(int buf_size)
/*=================================*/
{
  AutoScaler *t = (AutoScaler *)calloc(sizeof(AutoScaler), 1) ;
  t->buf_size = buf_size ;
  t->min_list = (int *)malloc(buf_size*sizeof(int)) ;
  t->max_list = (int *)malloc(buf_size*sizeof(int)) ;
  t->buffer = (double *)malloc(buf_size*sizeof(double)) ;
  return t ;
  }

void autoscaler_free(AutoScaler *t)
/*================================*/
{
  free(t->min_list) ;
  free(t->max_list) ;
  free(t->buffer) ;
  free(t) ;
  }

static void get_minimum(AutoScaler *t, double d)
/*============================================*/
{
  if (t->min_list_count == 0) {
    t->min_list[t->min_list_lo] = t->buf_pos ;
    t->min_list_count = 1 ;
    }
  else if (d <= t->buffer[t->min_list[t->min_list_lo]]) {
    t->min_list[t->min_list_lo] = t->buf_pos ;
    t->min_list_hi = t->min_list_lo ;
    t->min_list_count = 1 ;
    }
  else if (d < t->buffer[t->min_list[t->min_list_hi]]) {
    int pos ;
    int hi = t->min_list_hi ;
    int lo = t->min_list_lo ;
    if (hi < lo) hi += t->buf_size ;
    while (lo <= hi) {
      int mid = (hi + lo)/2 ;
      pos = mid ;
      if (pos >= t->buf_size) pos -= t->buf_size ;
      if      (d > t->buffer[t->min_list[pos]]) lo = pos + 1 ;
      else if (d < t->buffer[t->min_list[pos]]) hi = pos - 1 ;
      else                                      break ;
      }
    if (d == t->buffer[t->min_list[pos]]) t->min_list_hi = pos ;
    else {
      if (lo >= t->buf_size) lo -= t->buf_size ;
      t->min_list[lo] = t->buf_pos ;
      t->min_list_hi = lo ;
      }
    t->min_list_count = t->min_list_hi - t->min_list_lo + 1 ;
    if (t->min_list_count <= 0) t->min_list_count += t->buf_size ;
    }
  else {
    t->min_list[++t->min_list_hi] = t->buf_pos ;
    if (t->min_list_count < t->buf_size) ++t->min_list_count ;
    else                            ++t->min_list_lo ;
    if (t->min_list_lo >= t->buf_size) t->min_list_lo = 0 ;
    if (t->min_list_hi >= t->buf_size) t->min_list_hi = 0 ;
    }
  if (t->buf_count < t->buf_size)
    t->min_total += t->buffer[t->min_list[t->min_list_lo]] ;
  else
    t->min_total += (t->buffer[t->min_list[t->min_list_lo]] - (double)t->min_total/t->buf_count) ;

//  printf("Min: %f %f %d\n", t->buffer[t->min_list[t->min_list_lo]], t->min_total, t->buf_count) ;
  }

static void get_maximum(AutoScaler *t, double d)
/*============================================*/
{
  if (t->max_list_count == 0) {
    t->max_list[t->max_list_hi] = t->buf_pos ;
    t->max_list_count = 1 ;
    }
  else if (d >= t->buffer[t->max_list[t->max_list_hi]]) {
    t->max_list[t->max_list_hi] = t->buf_pos ;
    t->max_list_lo = t->max_list_hi ;
    t->max_list_count = 1 ;
    }
  else if (d > t->buffer[t->max_list[t->max_list_lo]]) {
    int pos ;
    int hi = t->max_list_lo ;
    int lo = t->max_list_hi ;
    if (hi < lo) hi += t->buf_size ;
    while (lo <= hi) {
      int mid = (hi + lo)/2 ;
      pos = mid ;
      if (pos >= t->buf_size) pos -= t->buf_size ;
      if      (d < t->buffer[t->max_list[pos]]) lo = pos + 1 ;
      else if (d > t->buffer[t->max_list[pos]]) hi = pos - 1 ;
      else                                      break ;
      }
    if (d == t->buffer[t->max_list[pos]]) t->max_list_lo = pos ;
    else {
      if (lo >= t->buf_size) lo -= t->buf_size ;
      t->max_list[lo] = t->buf_pos ;
      t->max_list_lo = lo ;
      }
    t->max_list_count = t->max_list_lo - t->max_list_hi + 1 ;
    if (t->max_list_count <= 0) t->max_list_count += t->buf_size ;
    }
  else {
    t->max_list[++t->max_list_lo] = t->buf_pos ;
    if (t->max_list_count < t->buf_size) ++t->max_list_count ;
    else                            ++t->max_list_hi ;
    if (t->max_list_hi >= t->buf_size) t->max_list_hi = 0 ;
    if (t->max_list_lo >= t->buf_size) t->max_list_lo = 0 ;
    }
  if (t->buf_count < t->buf_size)
    t->max_total += t->buffer[t->max_list[t->max_list_hi]] ;
  else
    t->max_total += (t->buffer[t->max_list[t->max_list_hi]] - (double)t->max_total/t->buf_count) ;

//  printf("Max: %f %f %d\n", t->buffer[t->max_list[t->max_list_hi]], t->max_total, t->buf_count) ;
  }

double autoscale(AutoScaler *t, double d)
/*=====================================*/
{
  t->buffer[t->buf_pos] = d ;
  if (t->buf_count < t->buf_size) ++t->buf_count ;
  get_minimum(t, d) ;
  get_maximum(t, d) ;
  if (++t->buf_pos >= t->buf_size) t->buf_pos = 0 ;
  return (t->max_total != t->min_total)
          ? (d*t->buf_count - t->min_total)/(t->max_total - t->min_total) - 0.5
          : 0.0 ;
  }



int main(int argc, char **argv)
/*===========================*/
{
#define WINDOWLEN 1000  
#define CHANNELS     2

  AutoScaler **scalers = (AutoScaler **)calloc(sizeof(AutoScaler *), CHANNELS) ;
  AutoScaler **sp ;
  int n ;
  double data[CHANNELS] ;
  int running = 1 ;

  for (n = 0, sp = scalers ;  n < CHANNELS ;  ++n, ++sp) *sp = autoscaler(WINDOWLEN) ;

  while (running) {
    for (n = 0, sp = scalers ;  n < CHANNELS ;  ++n, ++sp) {
      double d ;
      running = (fscanf(stdin, "%lf", &d) > 0) ;
      if (running) data[n] = autoscale(*sp, d) ;
      else break ;
      }
    if (n == CHANNELS) {
      for (n = 0 ;  n < CHANNELS ;  ++n) printf("%lf ", data[n]) ;
      printf("\n") ;
      }
    }

  for (n = 0, sp = scalers ;  n < CHANNELS ;  ++n, ++sp) autoscaler_free(*sp) ;

  free(scalers) ;
  }
