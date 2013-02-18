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

#include <string.h>
#include <stdlib.h>

#ifndef _BSML_STRING_H
#define _BSML_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

/** Make a copy of a string.
  * @param s The string to copy.
  * @return A copy of the string in a newly allocated memory block.
  */
const char *bsml_string_copy(const char *s) ;

/** Make a copy of part of a string.
  * @param s The string to copy.
  * @param n The number of characters to copy from the string.
  * @return A copy of the first `n` character of the string, NUL
            terminated in a newly allocated memory block.
  */
const char *bsml_string_copy_len(const char *s, int n) ;

/** Concatenate two strings.
  * @param s The first string to concatenate.
  * @param t The string to concatenate to the end of the first string.
  * @return The concatenation of the two strings in a newly allocated memory block.
  */
const char *bsml_string_cat(const char *, const char *) ;

/** Free memory allocated to a string.
  * @param s The string to free.
  */ 
void bsml_string_free(const char *s) ;

#ifdef __cplusplus
  } ;
#endif

#endif
