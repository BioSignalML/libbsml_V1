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
  * Access a BioSignalML repository.
  */

#ifndef _BSML_REPOSITORY_H
#define _BSML_REPOSITORY_H

#include "bsml_model.h"
#include "bsml_rdfgraph.h"
#include "bsml_data.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSML_REPOSITORY_STREAM     "/stream/data/"
#define BSML_REPOSITORY_RECORDING  "/recording/"


/*! A BiosignalML repository. */
typedef struct bsml_Repository {
  const char *uri ;           //!< The repository's URI
  const char *host ;          //!< The hostrname of the remote repository
  int port ;                  //!< The TCP/IP port of the remote repository
  const char *metadata_end ;  //!< The repository's metadata endpoint
  bsml_rdfgraph *graph ;      //!< Metadata about the repository
  } bsml_repository ;


void bsml_repository_initialise(void) ;

void bsml_repository_finish(void) ;

/*!
 * Connect to a BioSignalML repository.
 *
 * @param uri The URI of the repository
 * @return A ::bsml_repository, or NULL if errors
 */ 
bsml_repository *bsml_repository_connect(const char *uri) ;

/*!
 * Close the connection and free a BioSignalML repository object.
 *
 * @param repo The ::bsml_repository to close
 */ 
void bsml_repository_close(bsml_repository *repo) ;


/*!
 * Get metadata from a repository describing an object.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the object
 * @return A ::bsml_rdf_graph about the object, or NULL if errors
 */ 
bsml_rdfgraph *bsml_repository_get_metadata(bsml_repository *repo, const char *uri) ;


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


typedef void (TimeseriesProcess)(bsml_timeseries *ts) ;

void bsml_repository_get_timeseries(bsml_repository *repo, const char *data_uri,
                                    double start, double duration, TimeseriesProcess process) ;


#ifdef __cplusplus
  } ;
#endif

#endif
