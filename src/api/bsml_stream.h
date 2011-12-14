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

#ifndef _BSML_STREAM_H
#define _BSML_STREAM_H

#include "bsml_model.h"

#define BSML_STREAM_VERSION "1.0"

#ifdef __cplusplus
extern "C" {
#endif

bsml_recording *bsml_stream_recording_init(const char *, char, const char *, dict *) ;
void bsml_stream_recording_close(bsml_recording *) ;
bsml_recording *bsml_stream_recording_open(const char *, char, const char *) ;
bsml_recording *bsml_stream_recording_create(const char *, char, const char *, dict *) ;
bsml_recording *bsml_stream_recording_create_from_recording(bsml_recording *, const char *, char) ;

bsml_signal **bsml_stream_recording_create_signals(bsml_recording *, char **, bsml_timeseries **) ;
bsml_signal **bsml_stream_recording_create_signal_group(bsml_recording *, char **, double **, int, dict *) ;
void bsml_stream_recording_append_signal_data(bsml_recording *, double **, int) ;

void bsml_stream_recording_save_metadata(bsml_recording *, const char *, dict *) ;


bsml_signal *bsml_stream_signal_init(const char *, bsml_recording *, hid_t, dict *, int) ;
void bsml_stream_signal_close(bsml_signal *s) ;
bsml_signal *bsml_stream_signal_open(const char *, bsml_recording *r) ;
bsml_signal *bsml_stream_signal_create(const char *, bsml_recording *, bsml_timeseries *, dict *) ;
bsml_signal *bsml_stream_signal_create_from_signal(bsml_signal *, bsml_recording *) ;

void bsml_stream_signal_append(bsml_signal *, bsml_timeseries *) ;

#ifdef __cplusplus
} ;
#endif

#endif
