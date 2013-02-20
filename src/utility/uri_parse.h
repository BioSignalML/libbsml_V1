// Wrapper for uriparser from http://uriparser.sourceforge.net

#include <uriparser/Uri.h>


typedef struct {
  UriParserStateA state ;
  UriUriA uri ;
  const char *scheme ;
  int scheme_len ;
  const char *host ;
  int host_len ;
  int port ;
  } parsed_uri ;


parsed_uri *get_parsed_uri(const char *uri) ;

void parsed_uri_free(parsed_uri *u) ;

