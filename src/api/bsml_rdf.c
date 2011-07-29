/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdlib.h>
#include <sys/param.h>
#include <redland.h>

#include "bsml_rdf.h"

librdf_world *world = NULL ;


void GraphStore_free(GraphStore *g)
/*===============================*/
{
  if (g) {
    if (g->model) librdf_free_model(g->model) ;
    if (g->storage) librdf_free_storage(g->storage) ;
    }
  }

GraphStore *GraphStore_create(const char *path, const char *format)
/*===============================================================*/
{
  GraphStore *g = (GraphStore *)calloc(sizeof(GraphStore), 1) ;
  g->storage = librdf_new_storage(world, "hashes", "triples", "hash-type='memory'") ;
  g->model = librdf_new_model(world, g->storage, NULL) ;

  if (path) {
    librdf_uri *uri ;
    if (uri_protocol(path)) uri = librdf_new_uri(world, (const unsigned char *)path) ;
    else {
      char fullname[PATH_MAX+7] = "file://" ;
      realpath(path, fullname+7) ;
      uri = librdf_new_uri(world, (const unsigned char *)fullname) ;
      }
    if (format == NULL) format = "turtle" ;
    librdf_parser *parser = librdf_new_parser(world, format, NULL, NULL) ;
    if (librdf_parser_parse_into_model(parser, uri, uri, g->model)) {
      fprintf(stderr, "Failed to parse: %s\n", path) ;
      GraphStore_free(g) ;
      return NULL ;
      }
    librdf_free_parser(parser);
    librdf_free_uri(uri) ;
    }
  return g ;
  }
