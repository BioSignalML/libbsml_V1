/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdio.h>


#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "bsml_time.h"
#include "bsml_internal.h"
#include "utility/bsml_string.h"


bsml_time bsml_time_from_string(const char *d)
/*==========================================*/
{
  while (1) {      // We break on error else return
    if (*d++ != 'P') break ;
    const char *s = strchr(d, 'T') ;
    if (s == NULL) break ;
    int days = 0 ;
    int hours = 0 ;
    int mins = 0 ;
    int secs = 0 ;
    int usecs = 0 ;
    if (*(s-1) == 'D') {
      s = d ;
      while (isdigit(*s)) ++s ;
      if (*s != 'D') break ;
      days = (int)strtol(d, NULL, 10) ;
      if (*++s != 'T') break ;  // Could have two Ds
      }
    d = ++s ;
    while (isdigit(*s)) ++s ;
    if (*s == 'H') {
      hours = (int)strtol(d, NULL, 10) ;
      if (hours > 24) break ;
      d = ++s ;
      while (isdigit(*s)) ++s ;
      }
    if (*s == 'M') {
      mins = (int)strtol(d, NULL, 10) ;
      if (mins >= 60 || hours == 24 && mins != 0) break ;
      d = ++s ;
      while (isdigit(*s)) ++s ;
      }
    if (*s == 'S' || *s == '.') {
      secs = (int)strtol(d, NULL, 10) ;
      if (secs >= 60) break ;
      if (*s == '.') {
        d = ++s ;
        while (isdigit(*s)) ++s ;
        if (*s != 'S') break ;
        usecs = (int)strtol(d, NULL, 10) ;
        }
      d = ++s ;
      }
    if (*d) break ;  // Should be at end
    return bsml_time_from_seconds(86400.0*(double)days
                                 + 3600.0*(double)hours
                                 +   60.0*(double)mins
                                 +        (double)secs
                                 +        (double)usecs/1000000.0) ;
    }
  return (bsml_time)0 ;
  }


const char *bsml_time_as_string(bsml_time t)
/*========================================*/
{
  double seconds = bsml_time_as_seconds(t) ;
  long secs  = (long)seconds ;
  long usecs = (long)(1000000.0*(secs - seconds)) ;
  char days[16] ;
  if (secs >= 86400) snprintf(days, 16, "%ldD", secs/86400) ;
  else               *days = '\0' ;
  secs = secs % 86400 ;
  char hours[4] ;
  if (secs >= 3600) sprintf(hours, "%ldH", secs/3600) ;
  else              *hours = '\0' ;
  secs = secs % 3600 ;
  char mins[4] ;
  if (secs >= 60) sprintf(mins, "%ldM", secs/60) ;
  else            *mins = '\0' ;
  secs = secs % 60 ;
  char *iso = NULL ;
  if (usecs != 0) {
    asprintf(&iso, "P%sT%s%s%d.%06dS", days, hours, mins, secs, usecs) ;
    char *trailing = (iso + strlen(iso) - 2) ;  // Last digit
    while (*trailing == '0') --trailing ;
    if (*++trailing == '0') memcpy(trailing, "S", 2) ;
    }
  else
    asprintf(&iso, "P%sT%s%s%dS",      days, hours, mins, secs) ;
  return (const char *)iso ;
  }


static bsml_timestamp *bsml_timestamp_new(void)
/*===========================================*/
{
  return ALLOCATE(bsml_timestamp) ;
  }


bsml_timestamp *bsml_timestamp_get_utc(void)
/*========================================*/
{
  bsml_timestamp *ts = bsml_timestamp_new() ;
  struct timeval now ;
  gettimeofday(&now, NULL) ;
  gmtime_r(&now.tv_sec, &ts->datetime) ;
  ts->seconds = bsml_time_from_seconds((double)now.tv_usec/1000000.0) ;
  return ts ;
  }

bsml_timestamp *bsml_timestamp_get_local(void)
/*==========================================*/
{
  bsml_timestamp *ts = bsml_timestamp_new() ;
  struct timeval now ;
  gettimeofday(&now, NULL) ;
  localtime_r(&now.tv_sec, &ts->datetime) ;
  ts->seconds = bsml_time_from_seconds((double)now.tv_usec/1000000.0) ;
  return ts ;
  }

bsml_timestamp *bsml_timestamp_from_string(const char *s)
/*=====================================================*/
{
  bsml_timestamp *ts = bsml_timestamp_new() ;
  char *end = strptime(s, "%FT%T", &ts->datetime) ;
  while (end) {    // We break or return,,,
    if (*end == '.') {
      time_t usecs = strtol(end+1, &end, 10) ;
      ts->seconds = bsml_time_from_seconds((double)usecs/1000000.0) ;
      }
    if (*end == 'Z') {
      ++end ;
      ts->datetime.tm_gmtoff = 0 ;
      }
    else if (*end == '+' || *end == '-') { // Timezone offset
      int sign = (*end == '+') ? 1 : -1 ;
      int hh = (int)strtol(end, &end, 10) ;
      if (*end != ':') break ;
      int mm = (int)strtol(end, &end, 10) ;
      if (hh <= 13 && mm < 60 || hh == 14 && mm == 0)
        ts->datetime.tm_gmtoff = sign*60L*(60*hh + mm) ;
      else
        break ;
      }
    else {
      time_t clock ;
      time(&clock) ;
      struct tm *now = localtime(&clock) ;
      ts->datetime.tm_gmtoff = now->tm_gmtoff ;   // Set to local timezone
      }
    if (*end == '\0') return ts ;
    }
  free(ts) ;
  return NULL ;     // Errors
  }

const char *bsml_timestamp_as_string(bsml_timestamp *ts)
/*====================================================*/
{
  char buf[128] ;
  char *bufp = buf ;
  bufp += strftime(bufp, 100, "%FT%T", &ts->datetime) ;
  long usecs = (long)(1000000.0 * bsml_time_as_seconds(ts->seconds)) ;
  if (usecs) {
    sprintf(bufp, ".%06ld", usecs) ;
    bufp += strlen(bufp) - 1 ;
    while (*bufp == '0') --bufp ;
    *++bufp = '\0' ;
    }
  if (ts->datetime.tm_gmtoff == 0) strcpy(bufp, "Z") ;
  else {
    time_t clock ;
    time(&clock) ;
    struct tm *now = localtime(&clock) ;
    int tzsecs = ts->datetime.tm_gmtoff ;
    if (tzsecs != now->tm_gmtoff) {  // Non local timezone
      int hh = tzsecs/3600 ;
      tzsecs = tzsecs % 3600 ;
      int mm = tzsecs/60 ;
      if (mm < 0) mm = -mm ;
      sprintf(bufp, "%+03d:%02d", hh, mm) ;
      }
    }
  return bsml_string_copy(buf) ;
  }

void bsml_timestamp_add(bsml_timestamp *ts, bsml_time t)
/*====================================================*/
{
  ts->seconds += t ;
  }


void bsml_timestamp_normalise(bsml_timestamp *ts)
/*=============================================*/
{
  time_t secs = (time_t)bsml_time_as_seconds(ts->seconds) ;
  if (secs < 0) secs -= 1 ;  // As above rounding was towards zero
  time_t dtime = timegm(&ts->datetime) + secs ;
  gmtime_r(&dtime, &ts->datetime) ;
  ts->seconds -= bsml_time_from_seconds(secs) ;
  }


void bsml_timestamp_free(bsml_timestamp *ts)
/*========================================*/
{
  free(ts) ;
  }


#ifdef TEST_TIME

#include <stdio.h>

int main(void)

/*==========*/
{


  bsml_timestamp *now = bsml_timestamp_get_utc() ;
  const char *s = bsml_timestamp_as_string(now) ;
  printf("UTC: %s\n", s) ;
  bsml_string_free(s) ;
  bsml_timestamp_free(now) ;


  now = bsml_timestamp_get_local() ;
  s = bsml_timestamp_as_string(now) ;
  printf("NOW: %s\n", s) ;
  bsml_string_free(s) ;
  bsml_timestamp_free(now) ;


  }

#endif
