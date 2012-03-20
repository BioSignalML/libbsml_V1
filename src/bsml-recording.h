/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_RECORDING_H
#define _BSML_RECORDING_H

#ifdef __cplusplus
extern "C" {
#endif

/*! A BiosignalML recording. */
struct bsml_Recording {
  const char *uri ;           /*!< The recording's URI */
  bsml_rdfgraph *graph ;      /*!< Metadata about the recording */
  } ;


bsml_recording *bsml_recording_create_from_graph(const char *uri, bsml_rdfgraph *graph, bsml_repository *repo) ;

#ifdef __cplusplus
  } ;
#endif

#endif

