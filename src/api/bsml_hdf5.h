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

#ifndef _BSML_HDF5_H
#define _BSML_HDF5_H

#include "bsml_model.h"


#define HDF5_CHUNKSIZE 4096

typedef struct {
  char **uris ;
  int channels ;
  hid_t file ;
  hid_t recording ;
  hid_t sigdata ;
  } HDF5RecInfo ;

typedef struct {
  hid_t dataset ;
  int index ;
  } HDF5SigInfo ;


#ifdef __cplusplus
extern "C" {
#endif

Recording *HDF5Recording_init(const char *, char, const char *, Dictionary *) ;
void HDF5Recording_close(Recording *) ;
Recording *HDF5Recording_open(const char *, char, const char *) ;
Recording *HDF5Recording_create(const char *, char, const char *, Dictionary *) ;
Recording *HDF5Recording_create_from_recording(Recording *, const char *, char) ;

Signal **HDF5Recording_create_signals(Recording *, char **, TimeSeries **) ;
Signal **HDF5Recording_create_signal_group(Recording *, char **, double **, int, Dictionary *) ;
void HDF5Recording_append_signal_data(Recording *, double **, int) ;

void HDF5Recording_save_metadata(Recording *, const char *, Dictionary *) ;


Signal *HDF5Signal_init(const char *, Recording *, hid_t, Dictionary *, int) ;
void HDF5Signal_close(Signal *s) ;
Signal *HDF5Signal_open(const char *, Recording *r) ;
Signal *HDF5Signal_create(const char *, Recording *, TimeSeries *, Dictionary *) ;
Signal *HDF5Signal_create_from_signal(Signal *, Recording *) ;

void HDF5Signal_append(Signal *, TimeSeries *) ;

#ifdef __cplusplus
} ;
#endif

#endif
