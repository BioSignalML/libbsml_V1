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
  * Access BioSignalML signals.
  */

#ifndef _BSML_SIGNAL_H
#define _BSML_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Create a BioSignalML signal object.
 *
 * @param uri The URI of the signal
 * @return A ::bsml_signal, or NULL if errors
 */ 
bsml_signal *bsml_new_signal(const char *uri) ;

/*!
 * Free a BioSignalML signal object.
 *
 * @param repo The ::bsml_signal to free
 */ 
void bsml_free_signal(bsml_signal *sig) ;

/*!
 * Open a BioSignalML signal.
 *
 * We attempt to obtain RDF metadata by referencing the URI and use this
 * to create a signal object.
 *
 * @param uri The URI of the signal
 * @return A ::bsml_signal, or NULL if errors
 */ 
bsml_signal *bsml_signal_open(const char *uri) ;

#ifdef __cplusplus
  } ;
#endif

#endif
