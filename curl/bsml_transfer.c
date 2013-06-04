/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2013  David Brooks
 *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//#include <sys/errno.h>

#include <curl/curl.h>

// cc bsml_transfer.c -lcurl -o edf_transfer -DSTANDALONE

#include "bsml_transfer.h"


typedef struct {
  CURL *handle ;
  FILE *output ;
  char *request ;
  char *content ;
  char **error ;
  long status ;
  } RecvInfo ;


typedef struct {
  FILE *source ;
  char **error ;
  } SendInfo ;


static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
/*==========================================================================*/
{
  RecvInfo *info = (RecvInfo *)userp ;
  curl_easy_getinfo(info->handle, CURLINFO_RESPONSE_CODE, &info->status) ;
  if (info->status >= 200 && info->status < 300) {
    if (info->content == NULL) {
      curl_easy_getinfo(info->handle, CURLINFO_CONTENT_TYPE, &info->content) ;
      if (info->content != NULL && info->request != NULL
       && strcmp(info->content, info->request) != 0) {
        asprintf(info->error, "Content type doesn't match that requested") ;
        return 0 ;
        }
      }
    }
  else {
    asprintf(info->error, "%-.*s", size*nmemb, buffer) ;
    return 0 ;              // Will terminate easy_perform() with error result
    }
  if (info->output != NULL) return fwrite(buffer, size, nmemb, info->output) ;
  else                      return size*nmemb ;
  }


int bsml_to_file(const char *uri, const char *fname, const char *type, const char *key, char **error)
/*=================================================================================================*/
{
  *error = NULL ;

  FILE *output = fopen(fname, "wb") ;
  if (output == NULL) {
    asprintf(error, "Cannot create file") ;
    return 0 ;
    }

  CURL *handle = curl_easy_init() ;
  curl_easy_setopt(handle, CURLOPT_URL, uri) ;

  struct curl_slist *headers = NULL ;
  char *reqtype = NULL ;
  char *accept = NULL ;
  if (type == NULL) {
    headers = curl_slist_append(headers, "Accept: application/x-bsml") ;
    }
  else {
    asprintf(&reqtype, "application/x-bsml+%s", type) ;
    asprintf(&accept, "Accept: %s", reqtype) ;
    headers = curl_slist_append(headers, accept) ;
    }
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers) ;

  char *access = NULL ;
  if (key != NULL) {
    asprintf(&access, "access=%s", key) ;
    curl_easy_setopt(handle, CURLOPT_COOKIE, access) ;
    }

  RecvInfo info = { handle, output, reqtype, NULL, error, 0 } ;
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data) ;
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &info) ;

  int result = curl_easy_perform(handle) ;

  fclose(output) ;
  if (access != NULL) free(access) ;
  curl_slist_free_all(headers) ;
  if (accept != NULL) free(accept) ;
  if (reqtype != NULL) free(reqtype) ;

  if (result == 0) {    // All well with transfer, check status
    long status = 0 ;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status) ;
    if (*error == NULL && (status < 200 || status >= 300))
      asprintf(error, "HTTP transfer error: %d", status) ;
    }

  curl_easy_cleanup(handle) ;
  if (*error != NULL) {  // Errors above...
    // Remove file...
    }
  return (*error == NULL) ;
  }


static size_t read_data(void *buffer, size_t size, size_t nmemb, void *userp)
/*=========================================================================*/
{
  SendInfo *info = (SendInfo *)userp ;
  size_t bytes = fread(buffer, size, nmemb, info->source) ;
  if (ferror(info->source)) {
    asprintf(info->error, "Error reading file: %s", strerror(errno)) ;
    return 0 ;
    }
  return bytes ;    // Will be zero at EOF
  }


int file_to_bsml(const char *fname, const char *uri, const char *type, const char *key, char **error)
/*=================================================================================================*/
{
  *error = NULL ;

  struct stat fstatus ;
  FILE *source = fopen(fname, "rb") ;
  if (source == NULL) {
    asprintf(error, "Cannot open file") ;
    return 0 ;
    }
  fstat(fileno(source), &fstatus) ;

  CURL *handle = curl_easy_init() ;
  curl_easy_setopt(handle, CURLOPT_URL, uri) ;

  char *readerror = NULL ;
  SendInfo sendinfo = { source, &readerror } ;
  curl_easy_setopt(handle, CURLOPT_READFUNCTION, &read_data) ;
  curl_easy_setopt(handle, CURLOPT_READDATA, &sendinfo);
  curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L) ;
  curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fstatus.st_size) ;

  struct curl_slist *headers = NULL ;
  char *content = NULL ;
  asprintf(&content, "Content-Type: application/x-bsml+%s", type) ;
  headers = curl_slist_append(headers, content) ;
  headers = curl_slist_append(headers, "Transfer-Encoding: chunked") ;
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers) ;

  char *access = NULL ;
  if (key != NULL) {
    asprintf(&access, "access=%s", key) ;
    curl_easy_setopt(handle, CURLOPT_COOKIE, access) ;
    }

  RecvInfo recvinfo = { handle, NULL, NULL, NULL, error, 0 } ;
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data) ;
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, &recvinfo) ;

  int result = curl_easy_perform(handle) ;

  fclose(source) ;
  if (access != NULL) free(access) ;
  curl_slist_free_all(headers) ;
  free(content) ;

  if (readerror != NULL) {
    if (*error != NULL) free(*error) ;
    *error = readerror ;
    }
  else if (result == 0) {    // All well with transfer, check status
    long status = 0 ;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status) ;
    if (*error == NULL && (status < 200 || status >= 300))
      asprintf(error, "HTTP transfer error: %d", status) ;
    }

  curl_easy_cleanup(handle) ;
  return (*error == NULL) ;
  }


#ifdef STANDALONE

int main(int argc, char **argv)
/*===========================*/
{

  if (argc < 4
   || (strcmp(argv[1], "get") != 0
    && strcmp(argv[1], "put") != 0)) {
    printf("Usage:\t%s get URI FILE [key]\n\t%s put FILE URI [key]\n", argv[0], argv[0]) ;
    exit(1) ;
    }

  char *key = NULL ;
  if (argc >= 5) key = argv[3] ;

  char *err = NULL ;
  curl_global_init(CURL_GLOBAL_DEFAULT) ;
  if (strcmp(argv[1], "get") == 0) bsml_to_file(argv[2], argv[3], "edf", key, &err) ;
  else                             file_to_bsml(argv[2], argv[3], "edf", key, &err) ;
  curl_global_cleanup() ;

  if (err != NULL) {
    printf("ERROR %s", (err != NULL) ? err : "") ;
    free(err) ;
    exit(1) ;
    }

  }

#endif
