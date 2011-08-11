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
#include "bsml_internal.h"

typedef enum {
  BSML_RECORDING = 1,
  BSML_RECORDING_RAW,
  BSML_RECORDING_EDF,
  BSML_RECORDING_HDF5
  } BSML_RECORDING_TYPES ;

typedef enum {
  BSML_SIGNAL = 1,
  BSML_SIGNAL_RAW,
  BSML_SIGNAL_EDF,
  BSML_SIGNAL_HDF5
  } BSML_SIGNAL_TYPES ;

/*
typedef struct {
  dict *metadata ;
  } bsml_resource ;
*/

typedef struct {
  int type ;
  const char *uri ;
  void *info ;
  dict *attributes ;
  bsml_graphstore *graph ;
  } bsml_recording ;


typedef struct {
  int type ;
  const char *uri ;
  void *info ;
  bsml_recording *recording ;
  dict *attributes ;
  } bsml_signal ;

typedef struct {
//  double starttime ; // or int64 sample_number; or Time starttime (and Time == int64 in nsec).
//  double duration ;
  long len ;
  double *data ;
  } bsml_timeseries ;

#ifdef __cplusplus
extern "C" {
#endif

list *bsml_model_recordings(bsml_graphstore *) ;

bsml_recording *bsml_recording_init(const char *, dict *, int type) ;
void bsml_recording_close(bsml_recording *) ;
dict *bsml_recording_get_metavars(bsml_recording *) ;
char *bsml_recording_metadata_as_string(bsml_recording *, const char *, dict *) ;
void *bsml_recording_add_signal(bsml_recording *, bsml_signal *) ;

bsml_signal *bsml_signal_init(const char *, dict *, int) ;
void bsml_signal_close(bsml_signal *) ;
dict *bsml_signal_get_metavars(bsml_signal *) ;


bsml_recording *bsml_file_recording_init(const char *, const char *, dict *, int) ;
void bsml_file_recording_close(bsml_recording *) ;

bsml_signal *bsml_file_signal_init(const char *, dict *, int) ;
void bsml_file_signal_close(bsml_signal *) ;

#ifdef __cplusplus
} ;
#endif

#endif
