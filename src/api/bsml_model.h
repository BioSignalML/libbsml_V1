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

#include "bsml_internal.h"

typedef enum {
  RECORDING_RAW = 1,
  RECORDING_EDF,
  RECORDING_HDF5
  } RECORDING_TYPES ;

typedef enum {
  SIGNAL_RAW = 1,
  SIGNAL_EDF,
  SIGNAL_HDF5
  } SIGNAL_TYPES ;

typedef struct {
  dict *metadata ;
  } Resource ;

typedef struct {
  int type ;
  char *uri ;
  void *info ;
  dict *attributes ;
  } Recording ;


typedef struct {
  int type ;
  char *uri ;
  void *info ;
  Recording *recording ;
  dict *attributes ;
  } Signal ;

typedef struct {
//  double starttime ; // or int64 sample_number; or Time starttime (and Time == int64 in nsec).
//  double duration ;
  long len ;
  double *data ;
  } TimeSeries ;

#ifdef __cplusplus
extern "C" {
#endif

Recording *Recording_init(const char *, dict *, int type) ;
void Recording_close(Recording *) ;
dict *Recording_get_metavars(Recording *) ;
char *Recording_metadata_as_string(Recording *, const char *, dict *) ;
void *Recording_add_signal(Recording *, Signal *) ;

Signal *Signal_init(const char *, dict *, int) ;
void Signal_close(Signal *) ;
dict *Signal_get_metavars(Signal *) ;


Recording *FILERecording_init(const char *, const char *, dict *, int) ;
void FILERecording_close(Recording *) ;

Signal *FILESignal_init(const char *, dict *, int) ;
void FILESignal_close(Signal *) ;

#ifdef __cplusplus
} ;
#endif

#endif
