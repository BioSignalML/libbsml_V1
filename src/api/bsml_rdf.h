/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_RDF_H
#define _BSML_RDF_H

#include <redland.h>

#include "bsml_names.h"
#include "bsml_rdfnames.h"

typedef struct {
  librdf_storage *storage ;
  librdf_model *model ;
  } GraphStore ;

#ifdef __cplusplus
extern "C" {
#endif

GraphStore *GraphStore_create(const char *, const char *) ;
void GraphStore_free(GraphStore *) ;

#ifdef __cplusplus
} ;
#endif

#endif
