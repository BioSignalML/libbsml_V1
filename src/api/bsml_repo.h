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

#ifndef _BSML_REPO_H
#define _BSML_REPO_H

#include "bsml_internal.h"


typedef struct bsml_repo bsml_repo ;


#ifdef __cplusplus
extern "C" {
#endif

bsml_repo *bsml_repo_connect(const char *uri, const char *user, const char *pass) ;

void bsml_repo_close(bsml_repo *repo) ;


#ifdef __cplusplus
} ;
#endif

#endif


