/******************************************************
 *
 *  BioSignalML Project API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 ******************************************************
 */

#ifndef _BSML_H
#define _BSML_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct DataFrame {
  Recording *recording ;
  int64 number ;
  double *data ;
  }


#ifdef __cplusplus
} ;
#endif

#endif

