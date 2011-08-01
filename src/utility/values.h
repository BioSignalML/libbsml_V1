/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _VALUES_H
#define _VALUES_H

typedef enum {
  TYPE_POINTER = 1,
  TYPE_STRING,
  TYPE_INTEGER,
  TYPE_REAL
  } VALUE_TYPE ;

typedef struct Value Value ;
typedef void (Value_Free)(void *) ;

#if BSML_INTERNALS
struct Value {
  VALUE_TYPE type ;
  union {
    void *pointer ;
    const char *string ;       // So we don't have to use 'pointer' with a cast
    long integer ;
    double real ;
    } ;
  int pointerkind ;
  Value_Free *delete ;         // How to free a pointer
  } ;
#endif

#ifdef __cplusplus
extern "C" {
#endif

const char *string_copy(const char *) ;

void value_free(Value *) ;

VALUE_TYPE  value_type(Value *) ;
void       *value_get_pointer(Value *, int *) ;
const char *value_get_string(Value *) ;
long        value_get_integer(Value *) ;
double      value_get_real(Value *) ;

#ifdef __cplusplus
} ;
#endif

#endif

