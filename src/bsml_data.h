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
  * Timeseries data.
  */

#ifndef _BSML_DATA_H
#define _BSML_DATA_H

#include "bsml_time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bsml_timeseries {
  const char *uri ;
  bsml_time start ;
  int length ;
  double rate ;
  double *clock ;
  double *data ;
  } bsml_timeseries ;


bsml_timeseries *bsml_timeseries_alloc(
  const char *uri,
  bsml_time start,
  int length,
  double rate,
  double *clock,
  double *data) ;

void bsml_timeseries_free(bsml_timeseries *ts) ;

int bsml_timeseries_length(bsml_timeseries *ts) ;

bsml_time bsml_timeseries_time(bsml_timeseries *ts, int i) ;

double bsml_timeseries_data(bsml_timeseries *ts, int i) ;

#ifdef __cplusplus
  } ;
#endif

#endif

