#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>



size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
/*===================================================================*/
{
  return fwrite(buffer, size, nmemb, (FILE *)userp) ;
  }


int biosignalml_to_edf(const char *uri, const char *fname, const char *access_key)
/*==============================================================================*/
{
  FILE *edfout = fopen(fname, "wb") ;
  if (fname == NULL) return 0 ;

  CURL *handle = curl_easy_init() ;
  char error_message[CURL_ERROR_SIZE] ;

  curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_message) ;
  curl_easy_setopt(handle, CURLOPT_URL, uri) ;
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data) ;
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, edfout) ;
  struct curl_slist *headers=NULL ;
  headers = curl_slist_append(headers, "Accept: application/x-bsml+edf") ;
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers) ;
  char *key = NULL ;
  if (access_key != NULL) {
    asprintf(&key, "access=%s", access_key) ;
    curl_easy_setopt(handle, CURLOPT_COOKIE, key) ;
    }
  int result = curl_easy_perform(handle) ;
  if (key != NULL) free(key) ;
  curl_slist_free_all(headers) ;
  curl_easy_cleanup(handle) ;

  fclose(edfout) ;
  if (result != 0) {
    fprintf(stderr, "libcurl: %s\n", error_message) ;
    // Remove file...
    }
  return (result == 0) ;
  }



int main(int argc, char **argv)
/*===========================*/
{

  if (argc < 3) {
    printf("Usage: %s URI FILE\n", argv[0]) ;
    exit(1) ;
    }

  curl_global_init(CURL_GLOBAL_DEFAULT) ;

  if (!biosignalml_to_edf(argv[1], argv[2], NULL)) {
    printf("Cannot transfer file\n") ;
    exit(1) ;
    }

  }

