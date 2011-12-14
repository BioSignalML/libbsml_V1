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

#ifndef _BSML_MODEL_H
#define _BSML_MODEL_H

#include "bsml_rdf.h"
#include "bsml_repo.h"
#include "bsml_internal.h"

typedef enum {
  BSML_RECORDING = 1,
  BSML_RECORDING_RAW,
  BSML_RECORDING_EDF,
  BSML_RECORDING_HDF5
  } BSML_RECORDING_TYPES ;

/*
typedef struct {
  dict *metadata ;
  } bsml_resource ;
*/

typedef struct {
  int type ;
  const char *uri ;
  const char *location ;
  void *info ;
  dict *attributes ;
  bsml_graph *graph ;
  } bsml_recording ;


typedef struct {
  int len ;
  long offset ;
  double rate ;
  double period ;
  const char *clock ;
  double *data ;
  } bsml_timeseries ;


typedef struct {
  const char *uri ;
  const char *location ;
  void *info ;
  bsml_recording *recording ;
  dict *attributes ;
  bsml_timeseries *data ;
  } bsml_signal ;


#ifdef __cplusplus
extern "C" {
#endif

/*! Setup and initialise globals.
 *
 *  @param repo URL to a repositories BioSignalML end-point.
 */
void bsml_model_initialise(const char *repo) ;
void bsml_model_finish(void) ;


list *bsml_model_recordings(bsml_graph *) ;

// Abstract Recordings
bsml_recording *bsml_recording_create(const char *, dict *, bsml_graph *, int) ;
bsml_recording *bsml_recording_open_uri(const char *, const char *) ;
void bsml_recording_free(bsml_recording *) ;

dict *bsml_recording_get_metavars(bsml_recording *) ;
char *bsml_recording_metadata_as_string(bsml_recording *, const char *, dict *) ;
void *bsml_recording_add_signal(bsml_recording *, bsml_signal *) ;


// Abstract Signals
bsml_signal *bsml_signal_create(const char *, dict *) ;
bsml_signal *bsml_signal_open_uri(const char *, const char *) ;
void bsml_signal_free(bsml_signal *) ;

dict *bsml_signal_get_metavars(bsml_signal *) ;
double bsml_signal_data_point(bsml_signal *, long, double, int *) ;


// Timeseries...
bsml_timeseries *bsml_timeseries_create(double *) ;
void bsml_timeseries_free(bsml_timeseries *) ;
double bsml_timeseries_data(bsml_timeseries *, const char *, long, double, int *) ;


// Recordings and Signals in files...
bsml_recording *bsml_file_recording_init(const char *, const char *, dict *, int) ;
void bsml_file_recording_close(bsml_recording *) ;

bsml_signal *bsml_file_signal_init(const char *, dict *) ;
void bsml_file_signal_close(bsml_signal *) ;

#ifdef __cplusplus
} ;
#endif

#endif
