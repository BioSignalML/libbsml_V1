/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/


#include "bsml_data.h"
#include "bsml_time.h"
#include "bsml_internal.h"


bsml_timeseries *bsml_timeseries_alloc(bsml_time start, int length,
/*===============================================================*/
                          double rate, double *clock, double *data)
{
  bsml_timeseries *ts = ALLOCATE(bsml_timeseries) ;
  ts->start = start ;
  ts->length = length ;
  ts->rate = rate ;
  ts->clock = clock ;
  ts->data = data ;
  return ts ;
  }

void bsml_timeseries_free(bsml_timeseries *ts)
/*==========================================*/
{
  if (ts) {
    if (ts->clock) free(ts->clock) ;
    if (ts->data) free(ts->data) ;
    free(ts) ;
    }
  }

int bsml_timeseries_length(bsml_timeseries *ts)
/*===========================================*/
{
  return ts->length ;
  }

bsml_time bsml_timeseries_time(bsml_timeseries *ts, int i)
/*======================================================*/
{
  if (i < 0 || i >= ts->length)
    bsml_log_error('INVALID INDEX for timeseries: %d\n', i) ;
  else if (ts->clock)
    return bsml_time_from_seconds(ts->clock[i]) ;
  else
    return bsml_time_from_seconds((double)i/ts->rate) ;
  }


double bsml_timeseries_date(bsml_timeseries *ts, int i)
/*===================================================*/
{
  if (i < 0 || i >= ts->length)
    bsml_log_error('INVALID INDEX for timeseries: %d\n', i) ;
  else
    return ts->data[i] ;
  }
