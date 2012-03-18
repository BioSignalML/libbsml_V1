/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_INTERNAL_H
#define _BSML_INTERNAL_H

#include "biosignalml.h"

#include "bsml-utility.h"
#inclide "bsml-data.h"
#include "bsml-graph.h"
#include "bsml-signal.h"
#include "bsml-recording.h"
#include "bsml-repository.h"
#include "bsml-stream.h"


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


/*! A BiosignalML recording. */
struct bsml_Recording {
  const char *uri ;           /*!< The recording's URI */
  bsml_rdfgraph *graph ;      /*!< Metadata about the recording */
  } ;


/*! A BiosignalML signal. */
struct bsml_Signal {
  const char *uri ;           /*!< The signal's URI */
  bsml_rdfgraph *graph ;      /*!< Metadata about the signal */
  } ;


#ifdef __cplusplus
  } ;
#endif

#endif
