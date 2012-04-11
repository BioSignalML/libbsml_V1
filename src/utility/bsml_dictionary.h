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
  * An associative dictionary in C.
  * Dictionary elements are chained in a single-linked list.
  */

#include <string.h>
#include <stdlib.h>

#ifndef _BSML_DICTIONARY_H
#define _BSML_DICTIONARY_H

#ifdef __cplusplus
extern "C" {
#endif

/** Types of things that can be stored.
  */
typedef enum {
  DICTIONARY_TYPE_LONG = 1,             //!< Long integers
  DICTIONARY_TYPE_DOUBLE,               //!< Double precision numbers
  DICTIONARY_TYPE_STRING,               //!< C strings
  DICTIONARY_TYPE_POINTER               //!< Arbitrary pointers
  } DICTIONARY_VALUE_TYPE ;


/** A function to free memory referenced by a pointer.
  */
typedef void (FreePointer)(void *) ;

/** A single element in a dictionary.
  */
typedef struct bsml_dictionary_element {
  const char *key ;                     //!< The element's name
  DICTIONARY_VALUE_TYPE type ;          //!< The type of value
  union {
    long lvalue ;                       //!< The value as a long
    double dvalue ;                     //!< The value as a double
    const char *svalue ;                //!< The value as a string
    void *pointer ;                     //!< The value as a pointer
    } ;
  FreePointer *pfree ;                  //!< How to free the pointer
  struct bsml_dictionary_element *next ; //!< The next element
  } bsml_dictionary_element ;

/** An associative dictionary.
  */
typedef struct bsml_dictionary {
  int length ;                          //!< The number of items in the dictionary
  int usecount ;                        //!< The number of copies of the dictionary
  bsml_dictionary_element *elements ;   //!< The first element in the dictionary
  } bsml_dictionary ;


/** Create a new dictionary.
  */
bsml_dictionary *bsml_dictionary_create(void) ;

/** Make a copy of a dictionary.
  * @param d The dictionary to copy.
  *
  * The dictionary is not actually copied; instead its use count is incremented.
  */
bsml_dictionary *bsml_dictionary_copy(bsml_dictionary *d) ;

/** Free a dictionary's memory.
  * @param d The dictionary to free.
  *
  * The dictionary's use count is decremented, and memory is deallocated only if it becomes zero.
  */
void bsml_dictionary_free(bsml_dictionary *d) ;


/** Get the number of elements in a dictionary.
  * @param d The dictionary.
  * @return The number of elements.
  */
int bsml_dictionary_length(bsml_dictionary *d) ;


/** A function called on each dictionary element when iterating.
  * @param e The current dictionary element.
  * @param p An optional parameter.
  */
typedef void (IterateDictionary)(bsml_dictionary_element *e, void *p) ;

/** A function called on each dictionary element when iterating that can stop the process
  * by returning a non-zero value.
  * @param e The current dictionary element.
  * @param p An optional parameter.
  * @return Non-zero to stop iterating.
  */
typedef int  (IterateDictionaryBreak)(bsml_dictionary_element *e, void *p) ;

/** Iterate over all elements in a dictionary.
  * @param d The dictionary to iterate over.
  * @param f The function to call on each element.
  * @param p A parameter to pass to the element iterator function.
  */
void bsml_dictionary_iterate(bsml_dictionary *d, IterateDictionary *f, void *p) ;

/** Iterate over elements in a dictionary, optionally terminating early.
  * @param d The dictionary to iterate over.
  * @param f The function to call on each element; a non-zero result will stop the iteration loop.
  * @param p A parameter to pass to the element iterator function.
  */
void bsml_dictionary_iterate_break(bsml_dictionary *d, IterateDictionaryBreak *f, void *p) ;


/** Add or replace a long number in a dictionary.
  * @param d The dictionary to insert into.
  * @param key The name of the element to insert.
  * @param v The number to insert.
  *
  * If the dictionary already contains an element with the key then its value is updated.
  */
void bsml_dictionary_set_long(bsml_dictionary *d, const char *key, long v) ;

/** Add or replace a double precision number in a dictionary.
  * @param d The dictionary to insert into.
  * @param key The name of the element to insert.
  * @param v The number to insert.
  *
  * If the dictionary already contains an element with the key then its value is updated.
  */
void bsml_dictionary_set_double(bsml_dictionary *d, const char *key, double v) ;

/** Add or replace a string in a dictionary.
  * @param d The dictionary to insert into.
  * @param key The name of the element to insert.
  * @param v The string to insert. A copy of the string is stored.
  *
  * If the dictionary already contains an element with the key then its value is updated.
  */
void bsml_dictionary_set_string(bsml_dictionary *d, const char *key, char *v) ;
/** Add or replace a string in a dictionary, without taking a copy.
  * @param d The dictionary to insert into.
  * @param key The name of the element to insert.
  * @param v The string to insert.
  *
  * If the dictionary already contains an element with the key then its value is updated.
  */
void bsml_dictionary_set_copied_string(bsml_dictionary *d, const char *key, char *v) ;

/** Add or replace an arbitrary pointer in a dictionary.
  * @param d The dictionary to insert into.
  * @param key The name of the element to insert.
  * @param v The pointer to insert.
  * @param pfree The function to call to free the memory referenced by the pointer. NULL
  *              means not to free the pointer when deallocting the element. 
  *
  * If the dictionary already contains an element with the key then its value is updated.
  */
void bsml_dictionary_set_pointer(bsml_dictionary *d, const char *key, void *v, FreePointer *pfree) ;


/** Find a dictionary element by its name.
  * @param d The dictionary to search.
  * @param key The name to lookup.
  * @return A dictionary element if the key exists, otherwise NULL.
  */
bsml_dictionary_element *bsml_dictionary_element_find(bsml_dictionary *d, const char *key) ;

/** Retrieve a long number stored in a dictionary.
  * @param d The dictionary to search.
  * @param key The name to lookup.
  * @param v A pointer in which to store the number if the key references a long value.
  * @return One, if the name refered to a long value, otherwise zero.
  */
int bsml_dictionary_get_long(bsml_dictionary *d, const char *key, long *v) ;

/** Retrieve a double precision number stored in a dictionary.
  * @param d The dictionary to search.
  * @param key The name to lookup.
  * @param v A pointer in which to store the number if the key references a double precision value.
  * @return One, if the name refered to a double precision value, otherwise zero.
  */
int bsml_dictionary_get_double(bsml_dictionary *d, const char *key, long *v) ;

/** Retrieve a string stored in a dictionary.
  * @param d The dictionary to search.
  * @param key The name to lookup.
  * @return The string, if the name refered to a string, otherwise NULL.
  */
const char *bsml_dictionary_get_string(bsml_dictionary *d, const char *key) ;

/** Retrieve a pointer stored in a dictionary.
  * @param d The dictionary to search.
  * @param key The name to lookup.
  * @return The pointer, if the name refered to a pointer, otherwise NULL.
  */
void *bsml_dictionary_get_pointer(bsml_dictionary *d, const char *key) ;

/** Delete an item from a dictionary.
  * @param d The dictionary to remove an item from.
  * @param key The name of the element to delete.
  */
void bsml_dictionary_delete(bsml_dictionary *d, const char *key) ;

#ifdef __cplusplus
  } ;
#endif

#endif

