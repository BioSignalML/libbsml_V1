#include <stdio.h>
#include <stdlib.h>

#include "bsml_model.h"



#define MAX_POINTS  2000    // ****************************


/**

 Have simulation specify entire duration that data is required for so that
 we can optimise requests and queue responses...

 Best to ask server for entire block of data and have it stream data back in small
 chunks, using buffering in TCP layers.

 At server we need a separate thread per data request. Does web.py do this for us??


 */


bsml_timeseries *bsml_timeseries_create(double *data)
//===================================================
{
  bsml_timeseries *ts = (bsml_timeseries *)calloc(sizeof(bsml_timeseries), 1) ;
  ts->data = data ;
  return ts ;
  }


void bsml_timeseries_free(bsml_timeseries *ts)
//============================================
{
  if (ts) {
    if (ts->data) free((void *)ts->data) ;
    if (ts->clock) free((void *)ts->clock) ;
    free((void *)ts) ;
    }
  }


double bsml_timeseries_data(bsml_timeseries *ts, const char *loc, long n, double t, int *eof)
//===========================================================================================
{
  if (n < ts->offset || n >= (ts->offset + ts->len)) {
    long m = MAX_POINTS*(n / MAX_POINTS) ;
    bsml_stream_read_timeseries(ts, loc, (double)m*ts->period, (double)MAX_POINTS*ts->period) ;
    // return with endianess corrected...
    // and always double?? NO, we specify what we want and convert??
    }

  if (n < ts->offset || n >= (ts->offset + ts->len)) {
    *eof = 1 ;
    return 0.0 ;
    }
  else {
    *eof = 0 ;
    return ts->data[n - ts->offset] ;
    }
  } 



double bsml_signal_data_point(bsml_signal *sig, long n, double t, int *eof)
//=========================================================================
{
  return bsml_timeseries_data(sig->data, sig->location, n, t, eof) ;
  }



