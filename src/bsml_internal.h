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

#ifdef __cplusplus
extern "C" {
#endif

/*! Shortcut for allocating storage of a given type. */
#define ALLOCATE(type) ((type *)calloc(1, sizeof(type)))

/*! Log an error message. */
void bsml_log_error(const char *format, ...) ;

int bsml_initialise(void) ;

void bsml_finish(void) ;

#ifdef __cplusplus
  } ;
#endif

#endif
