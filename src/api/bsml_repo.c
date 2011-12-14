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


#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>


#include "bsml_repo.h"


struct bsml_repo {

  const char *uri ;

  CURL *link ;

  } ;



bsml_repo *bsml_repo_connect(const char *uri, const char *user, const char *pass)
//===============================================================================
{

  bsml_repo *repo = (bsml_repo *)calloc(sizeof(bsml_repo), 1) ;

  if (repo) {
    repo->uri = string_copy(uri) ;

    curl_global_init(CURL_GLOBAL_ALL) ;
    repo->link = curl_easy_init() ;
    }
  return repo ;
  }


void bsml_repo_close(bsml_repo *repo)
//===================================
{
  if (repo) {
    if (repo->link) curl_easy_cleanup(repo->link) ;
    curl_global_cleanup() ;
    free(repo->uri) ;
    free(repo) ;
    }
  } 



static int request(bsml_repo *repo, const char *endpoint,
  const char *method, const char *body, dict *headers) ;

      response, content = self._http.request(self._href + endpoint, method=method, **kwds)


    except AttributeError:
      raise Exception("Can not connect to 4store -- check it's running")
    #logging.debug('Request -> %s', response)
    if response.status not in [200, 201]: raise Exception(content)
    return content

  def query(self, sparql, accept='application/xml'):
  #-------------------------------------------------
    #logging.debug('4s %s: %s', accept, sparql)
    try:
      return self._request('/sparql/', 'POST',
                           body=urllib.urlencode({'query': sparql}),
                           headers={'Content-type': 'application/x-www-form-urlencoded',
                                    'Accept': accept} )
    except Exception, msg:




bsml_repo_query(bsml_repo *repo, const char *sparql)
//==================================================
{
  if (repo && repo->link) {

    curl_easy_setopt(repo->link, CURLOPT_URL, "http://example.com") ;

    CURLcode res = curl_easy_perform(repo->link) ;

    }
  }
