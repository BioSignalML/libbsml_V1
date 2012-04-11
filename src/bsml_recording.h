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
  * Access BioSignalML recordings.
  */

#ifndef _BSML_RECORDING_H
#define _BSML_RECORDING_H

#include "bsml_units.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Create a BioSignalML recording object.
 *
 * @param uri The URI of the recording
 * @return A ::bsml_recording, or NULL if errors
 */ 
bsml_recording *bsml_new_recording(const char *uri) ;

/*!
 * Close and free a BioSignalML recording object.
 *
 * @param repo The ::bsml_recording to close and free.
 */ 
void bsml_recording_close(bsml_recording *rec) ;

/*!
 * Open a BioSignalML recording.
 *
 * We attempt to obtain RDF metadata by referencing the URI and use this
 * to create a recording object.
 *
 * @param uri The URI of the recording
 * @return A ::bsml_recording, or NULL if errors
 */ 
bsml_recording *bsml_recording_open(const char *uri) ;


bsml_signal *bsml_recording_new_signal_by_id(bsml_recording *rec, const char *id, BSML_UNITS units) ;

#ifdef __cplusplus
  } ;
#endif

#endif
