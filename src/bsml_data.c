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

#include "bsml_data.h"
#include "bsml_time.h"
#include "bsml_internal.h"
#include "utility/bsml_string.h"


bsml_timeseries *bsml_timeseries_alloc(const char *uri, bsml_time start, int length,
/*===============================================================================*/
                                       double rate, double *clock, double *data)
{
  bsml_timeseries *ts = ALLOCATE(bsml_timeseries) ;
  ts->uri = bsml_string_copy(uri) ;
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
    bsml_string_free(ts->uri) ;
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
  if (i < 0 || i >= ts->length) {
    bsml_log_error("INVALID INDEX for timeseries: %d\n", i) ;
    return 0.0 ;
    }
  else if (ts->clock)
    return ts->start + bsml_time_from_seconds(ts->clock[i]) ;
  else
    return ts->start + bsml_time_from_seconds((double)i/ts->rate) ;
  }


double bsml_timeseries_data(bsml_timeseries *ts, int i)
/*===================================================*/
{
  if (i < 0 || i >= ts->length) {
    bsml_log_error("INVALID INDEX for timeseries: %d\n", i) ;
    return 0.0 ;
    }
  else
    return ts->data[i] ;
  }
