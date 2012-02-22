/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BIOSIGNALML_INTERNAL_H
#define _BIOSIGNALML_INTERNAL_H

#include <redland.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! Shortcut for allocating storage of a given type. */
#define ALLOCATE(type) ((type *)calloc(1, sizeof(type)))


/*! Make a copy of a string. */
const char *string_copy(const char *s) ;

/*! Return a copy of the concatenation of two strings. */
const char *string_cat(const char *s, const char *t) ;

/*! Log an error message. */
void bsml_log_error(const char *format, ...) ;


/*! A RDF graph. */
struct bsml_RDF_graph {
  const char *uri ;           /*!< The graph's URI */
  librdf_storage *storage ;   /*!< Where the graph is stored */
  librdf_model *model ;       /*!< The statements in the graph */
  } ;


/*! A BiosignalML repository. */
struct bsml_Repository {
  const char *uri ;           /*!< The repository's URI */
  const char *metadata_uri ;  /*!< The repository's metadata endpoint */
  const char *stream_uri ;    /*!< The repository's stream endpoint */
  bsml_rdf_graph *graph ;     /*!< Metadata about the repository */
  } ;


/*! A BiosignalML recording. */
struct bsml_Recording {
  const char *uri ;           /*!< The recording's URI */
  bsml_rdf_graph *graph ;     /*!< Metadata about the recording */
  } ;


#ifdef __cplusplus
  } ;
#endif

#endif
