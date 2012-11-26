// TEST

#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>

typedef enum {
  HTTP_GET = 1,
  HTTP_PUT,
  HTTP_POST,
  HTTP_DELETE
  } HTTP_METHOD ;


#define method HTTP_POST




int main(void)
{

  curl_global_init(CURL_GLOBAL_ALL) ;

  CURL *curl = curl_easy_init() ;
  long status = 0 ;

  if (curl) {  // ** otherwise error...
    const char *md_uri = "http://devel.biosignalml.org/metadata/" ;
    const char *ctype = "Content-Type: application/rdf+xml" ;

    curl_easy_setopt(curl, CURLOPT_URL, md_uri) ;

    unsigned char *body = "<RDF xmlns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'></RDF>" ;

    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L) ;
    //curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L) ;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)body) ;
    if (method == HTTP_PUT) curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L) ;

    struct curl_slist *slist = NULL ;
    slist = curl_slist_append(slist, ctype) ;
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist) ;

    int res = curl_easy_perform(curl) ;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status) ;

    curl_slist_free_all(slist) ;

    curl_easy_cleanup(curl) ;
    }

  printf("Status: %ld", status) ;

  curl_global_cleanup() ;
  }

