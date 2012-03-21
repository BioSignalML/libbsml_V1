#ifndef _BSML_REPOSITORY_H
#define _BSML_REPOSITORY_H  1

#include "bsml-rdfgraph.h"


#ifdef __cplusplus
extern "C" {
#endif

#define REPOSITORY_STREAM   "/stream/data/"
#define REPOSITORY_METADATA "/metadata/"


typedef enum {
  HTTP_GET = 1,
  HTTP_PUT,
  HTTP_POST,
  HTTP_DELETE
  } HTTP_METHOD ;


/*! A BiosignalML repository. */
struct bsml_Repository {
  const char *uri ;           /*!< The repository's URI */
  const char *metadata_end ;  /*!< The repository's metadata endpoint */
  bsml_rdfgraph *graph ;      /*!< Metadata about the repository */
  } ;


void bsml_repository_initialise(void) ;

void bsml_repository_finish(void) ;


/*!
 * Get metadata from a repository describing an object.
 *
 * @param repo The ::bsml_repository to use
 * @param uri The URI of the object
 * @return A ::bsml_rdf_graph about the object, or NULL if errors
 */ 
bsml_rdfgraph *bsml_repository_get_metadata(bsml_repository *repo, const char *uri) ;

#ifdef __cplusplus
  } ;
#endif

#endif
