/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *****************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "bsml_internal.h"
#include "bsml_rdfgraph.h"
#include "bsml_repository.h"
#include "bsml_stream.h"


int bsml_initialise(void)
/*=====================*/
{
  bsml_rdfgraph_initialise() ;

  //bsml_rdfmapping_initialise() ;

  bsml_stream_initialise() ;

  bsml_repository_initialise() ;

  return 1 ;
  }



void bsml_finish(void)
/*===================*/
{
  bsml_repository_finish() ;

  bsml_stream_finish() ;

  //bsml_rdfmapping_finish() ;
  bsml_rdfgraph_finish() ;
  }
