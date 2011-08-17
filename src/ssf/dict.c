#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "cJSON/cJSON.h"
#include "dictionary.h"


enum {
  KIND_JSON_ARRAY = 100,
  KIND_JSON_OBJECT
  } ;


dict *cJSON_dictionary(cJSON *item)
//=================================
{
  if (item == NULL) return NULL ;

	dict *d = dict_create() ;
  
  item = item->child ;
  while (item) {

    const char *key = item->string ;
	  
    switch ((item->type) & 0xFF) {

		 case cJSON_NULL:
      dict_set_pointer(d, key, NULL, 0, NULL) ;
      break ;

		 case cJSON_False:
      dict_set_integer(d, key, 0) ;
      break ;

		 case cJSON_True:
      dict_set_integer(d, key, 1) ;
      break ;

     case cJSON_Number: {
      double dbl = item->valuedouble ;
      if (fabs(((double)item->valueint) - dbl) <= DBL_EPSILON
       && dbl <= INT_MAX && dbl >= INT_MIN) dict_set_integer(d, key, item->valueint) ;
      else                                  dict_set_real(d, key, dbl) ;
      break ;
      }

     case cJSON_String:
      dict_set_string(d, key, item->valuestring) ;
      break ;

     case cJSON_Array:
      dict_set_pointer(d, key, item->child, KIND_JSON_ARRAY, NULL) ;
      break ;

     case cJSON_Object:
      dict_set_pointer(d, key, item->child, KIND_JSON_OBJECT, NULL) ;
      break ;
      }
    item = item->next ;
    }
  return d ;
  }
