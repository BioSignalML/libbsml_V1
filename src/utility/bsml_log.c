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
#include <stdarg.h>

#include "bsml_log.h"


void bsml_log_error(const char *format, ...)
/*========================================*/
{
  va_list ap ;
  va_start(ap, format) ;
  vfprintf(stderr, format, ap) ;
  va_end(ap) ;
  }

