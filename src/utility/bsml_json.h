/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-201  David Brooks
 *
 *  $ID$
 *
 *****************************************************/
/** @file
  * Additional operations on C strings.
  */

#ifndef _BSML_JSON_H
#define _BSML_JSON_H

#include <jansson.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Get a string value from a JSON object.
  * @param obj A JSON object.
  * @param key The key of the string value.
  * @return The string value if the key exists and refers to a string, otherwise an error message
  * is written to 'stderr' and NULL is returned.
  */
const char *bsml_json_as_string(json_t *obj, const char *key) ;

/** Get an integer value from a JSON object.
  * @param obj A JSON object.
  * @param key The key of the integer value.
  * @return The integer value if the key exists and refers to an integer, otherwise an error message
  * is written to 'stderr' and 0 is returned.
  */
int bsml_json_as_integer(json_t *obj, const char *key) ;

/** Get a numeric value from a JSON object.
  * @param obj A JSON object.
  * @param key The key of the numeric value.
  * @return The numeric value if the key exists and refers to a number, otherwise an error message
  * is written to 'stderr' and 0.0 is returned.
  */
double bsml_json_as_number(json_t *obj, const char *key) ;

#ifdef __cplusplus
  } ;
#endif

#endif
