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
  * Core BioSignalML objects.
  */

#ifndef _BSML_MODEL_H
#define _BSML_MODEL_H

#include "bsml_units.h"
#include "bsml_time.h"
#include "utility/bsml_dictionary.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! A BioSignalML recording. */
typedef struct bsml_recording {
  const char *uri ;           //!< The recording's URI
  const char *label ;         //!< A descriptive label for the recording
  const char *description ;   //!< Text describing the recording

  bsml_timestamp starttime ;  //!< The date and time the recording began
  bsml_time duration ;        //!< The recording's duration

  bsml_dictionary *metadata ; //!< Other metadata about the recording
// bsml_rdfgraph rdfgraph ;
  } bsml_recording ;
//  attributes = [ 'source', 'format', 'comment', 'investigation',


/*! A BioSignalML signal. */
typedef struct bsml_signal {
  const char *uri ;           //!< The signal's URI
  const char *label ;         //!< A descriptive label for the signal
  const char *description ;   //!< Text describing the signal
  BSML_UNITS units ;          //!< The physical units of the signal's data
  double rate ;               //!< The sampling rate if the signal is uniformly sampled
  int index ;                 //!< The position of the signal in its recording
  bsml_recording *recording ; //!< The recording the signal is part of
  bsml_dictionary *metadata ; //!< Other metadata about the signal
  } bsml_signal ;
// 'clock',
// 'transducer', 'filter',
//                'minFrequency', 'maxFrequency', 'minValue', 'maxValue',
// Are the above generic ??? Or in extended metadata dictionary ??


/*! A BioSignalML event. */
typedef struct bsml_event {
  const char *uri ;           //!< The event's URI
  const char *description ;   //!< Text describing the event
  bsml_time start ;           //!< ???? From when? Recording start??
  bsml_time duration ;        //!< The event's duration
  bsml_recording *recording ; //!< The recording the event is part of
  bsml_dictionary *metadata ; //!< Other metadata about the event
  } bsml_event ;

//  attributes = [ 'factor', 'time', ]


/*! A BioSignalML annotation. */
typedef struct bsml_annotation {
  const char *uri ;           //!< The annotation's URI
  const char *description ;   //!< Text describing the annotation
  bsml_dictionary *metadata ; //!< Other metadata about the annotation
  } bsml_annotation ;


#ifdef __cplusplus
  } ;
#endif

#endif
