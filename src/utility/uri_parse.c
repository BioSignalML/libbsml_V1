// Wrapper for uriparser from http://uriparser.sourceforge.net

#include <stdio.h>
#include <uriparser/Uri.h>

#include "bsml_internal.h"
#include "uri_parse.h"


parsed_uri *get_parsed_uri(const char *uri)
/*=======================================*/
{
  parsed_uri *u = ALLOCATE(parsed_uri) ;
  if (u) {
    u->state.uri = &u->uri ;
    if (uriParseUriA(&u->state, uri) != URI_SUCCESS || u->uri.hostText.first == NULL) {
      parsed_uri_free(u) ;
      return NULL ;
      }
    u->scheme = u->uri.scheme.first ;
    u->scheme_len = u->uri.scheme.afterLast - u->uri.scheme.first ;
    u->host = u->uri.hostText.first ;
    u->host_len = u->uri.hostText.afterLast - u->uri.hostText.first ;
    u->port = 80 ;
    if (u->uri.portText.first != NULL)
      sscanf(u->uri.portText.first, "%d", &u->port) ;
    }
  return u ;
  }


void parsed_uri_free(parsed_uri *u)
/*===============================*/
{
  if (u) {
    uriFreeUriMembersA(&u->uri) ;
    free(u) ;
    }
  }
