/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BIOSIGNALML_H
#define _BIOSIGNALML_H


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************/

/*! A RDF graph. */
typedef struct bsml_RDFgraph bsml_rdfgraph ;



/*!
 * \defgroup signals BioSignalML Signals
 * @{
 */

/*! A BiosignalML signal. */
typedef struct bsml_Signal bsml_signal ;

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

/*!@}*/

/**************************************************************/

/*!
 * \defgroup recordings BioSignalML Recordings
 * @{
 */

/*! A BiosignalML recording. */
typedef struct bsml_Recording bsml_recording ;

/*!
 * Create a BioSignalML recording object.
 *
 * @param uri The URI of the recording
 * @return A ::bsml_recording, or NULL if errors
 */ 
bsml_recording *bsml_new_recording(const char *uri) ;

/*!
 * Free a BioSignalML recording object.
 *
 * @param repo The ::bsml_recording to free
 */ 
void bsml_free_recording(bsml_recording *rec) ;

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


bsml_signal *bsml_recording_new_signal_by_id(bsml_recording *rec, const char *id) ;

/*!@}*/

/**************************************************************/

/*!
 * \defgroup repositories BioSignalML Repositories
 * @{
 */

/*! A BiosignalML repository. */
typedef struct bsml_Repository bsml_repository ;

/*!
 * Connect to a BioSignalML repository.
 *
 * @param uri The URI of the repository
 * @return A ::bsml_repository, or NULL if errors
 */ 
bsml_repository *bsml_repository_connect(const char *uri) ;

/*!
 * Close the connection to a BioSignalML repository.
 *
 * @param repo The ::bsml_repository to close
 */ 
void bsml_repository_close(bsml_repository *repo) ;

/*!
 * Create a new recording in a BioSignalML repository.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the new recording
 * @return A ::bsml_recording, or NULL if errors
 */ 
bsml_recording *bsml_repository_new_recording(bsml_repository *repo, const char *uri) ;

/*!
 * Open a recording in a BioSignalML repository.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the recording
 * @return A ::bsml_recording, or NULL if errors
 */ 
bsml_recording *bsml_repository_get_recording(bsml_repository *repo, const char *uri) ;

/*!
 * Open a signal in a BioSignalML repository.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the signal
 * @return A ::bsml_signal, or NULL if errors
 */ 
bsml_signal *bsml_repository_get_signal(bsml_repository *repo, const char *uri) ;

/*!@}*/

/**************************************************************/


/**************************************************************/

/**************************************************************/


#ifdef __cplusplus
  } ;
#endif

#endif
