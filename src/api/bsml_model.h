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

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
  char *uri ;
  int signals ;
  } Recording ;


typedef struct {
  Recording *recording ;
  int number ;
  double rate ;
  char *label ;
  } Signal ;


typedef struct {
  Signal *signal ;
  double starttime ;  // or int64 sample_number; or Time starttime (and Time == int64 in nsec).
  double duration ;
  int length ;
  double *data ;
  } SignalData ;


Recording *bsml_open_recording(const char *uri) ;
Recording *bsml_create_recording(const char *uri) ;

bsml_recording_open/create ??

bsml_recording_add_signal

int bsml_recording_signals(Recording *)

Signal *bsml_recording_get_signal(Recording *recording, int signalno)

void bsml_free_recording(Recording *recording) ;


#ifdef __cplusplus
} ;
#endif

#endif

