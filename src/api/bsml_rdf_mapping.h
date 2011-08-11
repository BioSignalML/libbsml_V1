/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_RDF_MAPPING_H
#define _BSML_RDF_MAPPING_H

#include "bsml_rdf.h"
#include "bsml_internal.h"

typedef struct bsml_rdfmapping bsml_rdfmapping ;


#ifdef __cplusplus
extern "C" {
#endif

void bsml_rdfmapping_initialise(void) ;

bsml_rdfmapping *bsml_rdfmapping_create(dict *) ;
void bsml_rdfmapping_free(bsml_rdfmapping *) ;

void bsml_rdfmapping_save_attributes(bsml_rdfmapping *, bsml_graphstore *, dict *, const char *, const char *) ;
void bsml_rdfmapping_get_attributes(bsml_rdfmapping *, dict *, bsml_graphstore *, const char *, const char *) ;

#ifdef __cplusplus
} ;
#endif

#endif
