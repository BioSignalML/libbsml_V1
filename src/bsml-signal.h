/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _BSML_SIGNAL_H
#define _BSML_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*! A BiosignalML signal. */
struct bsml_Signal {
  const char *uri ;           /*!< The signal's URI */
  bsml_rdfgraph *graph ;      /*!< Metadata about the signal */
  } ;

#ifdef __cplusplus
  } ;
#endif

#endif

