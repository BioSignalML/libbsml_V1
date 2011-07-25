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
  Dictionary *metadata ;
  } Resource ;

typedef struct {
  int type ;
  char *uri ;
  void *info ;
  Dictionary *attributes ;
  } Recording ;


typedef struct {
  int type ;
  char *uri ;
  void *info ;
  Recording *recording ;
  Dictionary *attributes ;
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

struct {
  char *RAW ;
  char *HDF5 ;
  }
BSML = { "bsml:rawurl",
         "bsml:hdf5url"
       } ;

Dictionary *recording_get_metavars(Recording *) ;

char *recording_metadata_as_string(const char *, Dictionary *) ;


Recording *Recording_init(const char *, Dictionary *, int type) ;
void Recording_close(Recording *) ;

Signal *Signal_init(const char *, Dictionary *, int) ;
void Signal_close(Signal *) ;

Recording *FILERecording_init(const char *, const char *, Dictionary *, int) ;
void FILERecording_close(Recording *) ;

Signal *FILESignal_init(const char *, Dictionary *, int) ;
void FILESignal_close(Signal *) ;

#ifdef __cplusplus
} ;
#endif

#endif
