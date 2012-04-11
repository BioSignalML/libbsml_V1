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
  * Calendar time and temporal durations.
  */

#include <stdint.h>
#include <time.h>

#ifndef _BSML_TIME_H
#define _BSML_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

/** A temporal duration.
  *
  * Time durations are stored as seconds in unsigned 64-bit integers. The low-order
  * 30 bits contain the fractional component; the remaining 34 bits the whole seconds
  * as a signed number.
  *
  * This gives a maximum duration of +/- 272 years at roughly 1 nanosecond resolution.
  */
typedef int64_t bsml_time ;

/** Convert from bsml_time to seconds as a double-precision number. */
#define bsml_time_as_seconds(t)   (((double)t)/((double)(0x40000000)))  /* 0x40000000 == 2**30 */

/** Convert from seconds to a bsml_time. */
#define bsml_time_from_seconds(s) ((bsml_time)((double)s*(double)(0x40000000)))


/** Get duration from an ISO 8601 string.
  * @param d  A string formated as an ISO 8601 duration. 
  * @return The duration represented in the string as a bsml_time.
  */
bsml_time bsml_time_from_string(const char *d) ;

/** Return the duration in ISO 8601 format.
  */
const char *bsml_time_as_string(bsml_time t) ;


/** A point in Universal time.
  */
typedef struct bsml_timestamp {
  struct tm datetime ;        //!< A calendar date/time.
  bsml_time seconds ;         //!< An offset from the calendar time
  } bsml_timestamp ;


/** Get UTC time.
  * @return The current Universal time.
  */
bsml_timestamp *bsml_timestamp_get_utc(void) ;

/** Get local time.
  * @return The current local time.
  */
bsml_timestamp *bsml_timestamp_get_local(void) ;

/** Get time from an ISO 8601 formated string.
  * @return The time represented in the string.
  */
bsml_timestamp *bsml_timestamp_from_string(const char *s) ;

/** Return the time in ISO 8601 format.
  */
const char *bsml_timestamp_as_string(bsml_timestamp *ts) ;

/** Add a duration to a time point.
  * @param ts The time stamp to modify.
  * @param t  The duration.
  */
void bsml_timestamp_add(bsml_timestamp *ts, bsml_time t) ;

/** Normalise a time point.
  * @param ts The time stamp to normalise.
  *
  * The component 'datetime' field is adjusted by the 'seconds' field
  * so that the residual seconds are in the semi-open interval [ 0.0, 1.0 ).
  */
void bsml_timestamp_normalise(bsml_timestamp *ts) ;

/** Free memory allocated to a time point.
  * @param ts The time stamp to free.
  */
void bsml_timestamp_free(bsml_timestamp *ts) ;

#ifdef __cplusplus
  } ;
#endif

#endif
