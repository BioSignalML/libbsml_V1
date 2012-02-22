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

/*!
 * \defgroup rdf_graphs RDF Graphs
 * @{
 */

/*! A RDF graph. */
typedef struct bsml_RDF_graph bsml_rdf_graph ;

/*! Initialise the RDF world. */
void bsml_rdf_initialise(void) ;

/*! Finish with the RDF world. */
void bsml_rdf_finish(void) ;

/*!
 * Create a rdf_graph object.
 *
 * @param uri The URI of the RDF graph
 * @return A ::bsml_rdf_graph, or NULL if errors
 */ 
bsml_rdf_graph *bsml_new_rdf_graph(const char *uri) ;

/*!
 * Free a rdf_graph object.
 *
 * @param graph The ::bsml_rdf_graph to free
 */ 
void bsml_free_rdf_graph(bsml_rdf_graph *graph) ;

/*!
 * Create a rdf_graph from RDF at an URI.
 *
 * @param uri The URI of the RDF graph
 * @param rdf_uri The URI of the graph's RDF
 * @return A ::bsml_rdf_graph, or NULL if errors
 */ 
bsml_rdf_graph *bsml_rdf_graph_create_and_load_rdf(const char *uri, const char *rdf_uri) ;

/*!@}*/

/**************************************************************/

/*!
 * \defgroup repositories BioSignalML Repositories
 * @{
 */

/*! A BiosignalML repository. */
typedef struct bsml_Repository bsml_repository ;

/*!
 * Create a repository object and load its metadata from a BioSignalML repository.
 *
 * @param uri The URI of the repository
 * @return A ::bsml_repository, or NULL if errors
 */ 
bsml_repository *bsml_new_repository(const char *uri) ;

/*!
 * Free a BioSignalML repository object.
 *
 * @param repo The ::bsml_repository to free
 */ 
void bsml_free_repository(bsml_repository *repo) ;

/*!
 * Get metadata from a repository describing an object.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the object
 * @return A ::bsml_rdf_graph about the object, or NULL if errors
 */ 
bsml_rdf_graph *bsml_repository_get_metadata(bsml_repository *repo, const char *uri) ;

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

/*!
 * Open a recording in a BioSignalML repository.
 *
 * We attempt to obtain RDF metadata from the repository for the URI and use this
 * to create a recording object.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the recording
 * @return A ::bsml_recording, or NULL if errors
 */ 
bsml_recording *bsml_recording_from_repository(bsml_repository *repo, const char *uri) ;

/*!@}*/

/**************************************************************/


#ifdef __cplusplus
  } ;
#endif

#endif
