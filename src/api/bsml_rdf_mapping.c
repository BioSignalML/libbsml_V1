/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>

#include <redland.h>

#include "bsml_internal.h"

/**

Role of mappings:

1) Take a dictionary of attribute/value pairs for some BSML resource
   and add them to a RDF model (or return a stream of RDF statements)
   
2) Given a RDF model and a uri of a BSML resource, return a dictionary
   of attribute/value pairs.    

**/



char *BSML_MAP_URI = "./mapping.ttl" ;

// 'file://' + os.path.dirname(os.path.abspath(__file__)) + '/mapping.ttl'


librdf_world *world = NULL ;


typedef struct {
  int sign ;
  struct tm datetime ;
  unsigned int usecs ;
  } DateTime ;

const char *datetime_to_isoformat(DateTime *dt)
/*===========================================*/
{
  char buf[128] ;
  char *bufp = buf ;
  if (dt->sign < 0) *bufp++ = '-' ;

  strftime(bufp, 100, "%FT%T", &dt->datetime) ;
  bufp += strlen(bufp) ;

  if (dt->usecs) {
    sprintf(bufp, ".%06d", dt->usecs) ;
    bufp += strlen(bufp) - 1 ;
    while (*bufp == '0') --bufp ;
    *++bufp = '\0' ;
    }

  if (dt->datetime.tm_gmtoff == 0) strcpy(bufp, "Z") ;
  else {
    time_t clock ;
    time(&clock) ;
    struct tm *now = localtime(&clock) ;
    int tzsecs = dt->datetime.tm_gmtoff ;
    if (tzsecs != now->tm_gmtoff) {  // Non local timezone
      int hh = tzsecs/3600 ;
      tzsecs = tzsecs % 3600 ;
      int mm = tzsecs/60 ;
      if (mm < 0) mm = -mm ;
      sprintf(bufp, "%+03d:%02d", hh, mm) ;
      }
    }
  return string_copy(buf) ;
  }


DateTime *isoformat_to_datetime(const char *str)
/*============================================*/
{
  DateTime *dt = (DateTime *)calloc(sizeof(DateTime), 1) ;
  dt->sign = 1 ;
  if (*str == '-') {
    ++str ;
    dt->sign = -1 ;
    }
  char *end = strptime(str, "%FT%T", &dt->datetime) ;

  if (end) {
    if (*end == '.') {
      ++end ;
      sscanf(end, "%d", &dt->usecs) ;
      while (isdigit(*end)) ++end ;
      }

    if (*end == 'Z') {
      ++end ;
      dt->datetime.tm_gmtoff = 0 ;
      }
    else if (*end == '+' || *end == '-') { // Timezone offset
      int sign = (*end == '+') ? 1 : -1 ;
      int hh = 0 ;
      int mm = 0 ;
      ++end ;
      if (sscanf(end, "%d:%d", &hh, &mm) == 2) {
        if (hh <= 13 && mm < 60 || hh == 14 && mm == 0)
          dt->datetime.tm_gmtoff = sign*60L*(60*hh + mm) ;
        while (isdigit(*end)) ++end ;
        if (*end == ':') {
          ++end ;
          while (isdigit(*end)) ++end ;
          }
        }
      }
    else {
      time_t clock ;
      time(&clock) ;
      struct tm *now = localtime(&clock) ;
      dt->datetime.tm_gmtoff = now->tm_gmtoff ;   // Set to local timezone
      }

    if (*end == '\0') return dt ;
    }

  free(dt) ;
  return NULL ;     // Errors
  }


const char *seconds_to_isoduration(double seconds)
/*==============================================*/
{
  long secs  = (long)seconds ;
  long usecs = (long)(1000000.0*(secs - (double)seconds)) ;
  char days[16] ;
  if (secs >= 86400) snprintf(days, 16, "%dD", secs/86400) ;
  else               *days = '\0' ;
  secs = secs % 86400 ;
  char hours[4] ;
  if (secs >= 3600) sprintf(hours, "%dH", secs/3600) ;
  else              *hours = '\0' ;
  secs = secs % 3600 ;
  char mins[4] ;
  if (secs >= 60) sprintf(mins, "%dM", secs/60) ;
  else            *mins = '\0' ;
  secs = secs % 60 ;

  char *iso = NULL ;
  if (usecs != 0) {
    asprintf(&iso, "P%sT%s%s%d.%06dS", days, hours, mins, secs, usecs) ;
    char *trailing = (iso + strlen(iso) - 2) ;  // Last digit
    while (*trailing == '0') --trailing ;
    if (*++trailing == '0') memcpy(trailing, "S", 2) ;
    }
  else
    asprintf(&iso, "P%sT%s%s%dS",      days, hours, mins, secs) ;

  return iso ;
  }


double isoduration_to_seconds(const char *d)
/*========================================*/
{
  while (1) {
    if (*d++ != 'P') break ;
    const char *s = strchr(d, 'T') ;
    if (s == NULL) break ;

    int days = 0 ;
    int hours = 0 ;
    int mins = 0 ;
    int secs = 0 ;
    int usecs = 0 ;
    if (*(s-1) == 'D') {
      s = d ;
      while (isdigit(*s)) ++s ;
      if (*s != 'D') break ;
      sscanf(d, "%d", &days) ;
      if (*++s != 'T') break ;  // Could have two Ds
      }
    d = ++s ;
    while (isdigit(*s)) ++s ;
    if (*s == 'H') {
      sscanf(d, "%d", &hours) ;
      if (hours > 24) break ;
      d = ++s ;
      while (isdigit(*s)) ++s ;
      }
    if (*s == 'M') {
      sscanf(d, "%d", &mins) ;
      if (mins >= 60 || hours == 24 && mins != 0) break ;
      d = ++s ;
      while (isdigit(*s)) ++s ;
      }
    if (*s == 'S' || *s == '.') {
      sscanf(d, "%d", &secs) ;
      if (secs >= 60) break ;
      if (*s == '.') {
        d = ++s ;
        while (isdigit(*s)) ++s ;
        if (*s != 'S') break ;
        sscanf(d, "%d", &usecs) ;
        }
      d = ++s ;
      }
    if (*d) break ;  // Should be at end

    return 86400.0*(double)days + 3600.0*(double)hours + 60.0*(double)mins
           + (double)secs + (double)usecs/1000000.0 ;
    }
  return 0.0 ;
  }


static unsigned char *QUERY_MAP = (unsigned char *)
  "prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>"
  "prefix map:  <http://www.biosignalml.org/ontologies/2011/02/mapping#>"
  "select ?lbl ?prop ?class ?otype ?map ?rmap"
  "  where {"
  "    ?m a map:Mapping ."
  "    ?m rdfs:label ?lbl ."
  "    ?m map:property ?prop ."
  "    optional { ?m map:class ?class }"
  "    optional { ?m map:object ?otype }"
  "    optional { ?m map:mapped-by ?map }"
  "    optional { ?m map:reverse-map ?rmap }"
  "    }" ;


typedef struct {
  librdf_uri *property ;
  librdf_uri *class ;
  librdf_uri *type ;
  const char *mapped ;
  const char *reverse ;
  } MapEntry ;


static librdf_uri *map_entry_get_uri(librdf_query_results *r, const char *k)
/*========================================================================*/
{
  librdf_uri *u = NULL ;
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_resource(n))
      u = librdf_new_uri_from_uri(librdf_node_get_uri(n)) ;
    librdf_free_node(n) ;
    }
  return u ;
  }

static const char *map_entry_get_string(librdf_query_results *r, const char *k)
/*===========================================================================*/
{
  const char *s = NULL ;
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_literal(n))
      s = string_copy((char *)librdf_node_get_literal_value(n)) ;
    librdf_free_node(n) ;
    }
  return s ;
  }

static void map_entry_free(void *p)
/*===============================*/
{
  MapEntry *m = p ;
  if (m->property) librdf_free_uri(m->property) ;
  if (m->class) librdf_free_uri(m->class) ;
  if (m->type) librdf_free_uri(m->type) ;
  if (m->mapped) free((void *)m->mapped) ;
  if (m->reverse) free((void *)m->reverse) ;
  free(m) ;
  }

static void mapping_set(dict *map, librdf_query_results *r)
/*=======================================================*/
{
  MapEntry *m = (MapEntry *)calloc(sizeof(MapEntry), 1) ;
  const char *key  = map_entry_get_string(r, "lbl") ;
  if (key) {
    m->property = map_entry_get_uri(r, "prop") ;
    m->class    = map_entry_get_uri(r, "class") ;
    m->type     = map_entry_get_uri(r, "otype") ;
    m->mapped   = map_entry_get_string(r, "map") ;
    m->reverse  = map_entry_get_string(r, "rmap") ;
    dict_set_pointer(map, key, m, map_entry_free) ;
    }
  }


static int uri_protocol(const char *u)
/*==================================*/
{
  return (memcmp(u, "file:", 5) == 0 || memcmp(u, "http:", 5) == 0) ;
  }


static dict *load_mapping(const char *mapfile)
/*==========================================*/
{

  librdf_storage *storage = librdf_new_storage(world, "hashes", "triples", "hash-type='memory'") ;
  librdf_model *model = librdf_new_model(world, storage, NULL) ;
  
  librdf_uri *uri ;
  if (uri_protocol(mapfile)) uri = librdf_new_uri(world, ( const unsigned char *)mapfile) ;
  else {
    char fullname[PATH_MAX+7] = "file://" ;
    realpath(mapfile, fullname+7) ;
    uri = librdf_new_uri(world, ( const unsigned char *)fullname) ;
    }

  librdf_parser *parser = librdf_new_parser(world, "turtle", NULL, NULL) ;
  if (librdf_parser_parse_into_model(parser, uri, uri, model)) {
    fprintf(stderr, "Failed to parse: %f\n", mapfile) ;
    return NULL ;
    }
  librdf_free_parser(parser);
  librdf_free_uri(uri) ;

  dict *mapping = dict_create() ;
  librdf_query *query = librdf_new_query(world, "sparql11-query", NULL, QUERY_MAP, NULL) ;
  librdf_query_results *results = librdf_model_query_execute(model, query) ;
  if (results == NULL) { //...
    fprintf(stderr, "SPARQL error...\n") ;
    }
  else {
    while (!librdf_query_results_finished(results)) {
      mapping_set(mapping, results) ;
      librdf_query_results_next(results) ;
      }
    librdf_free_query_results(results) ;
    }
  librdf_free_query(query) ;
  librdf_free_model(model) ;
  librdf_free_storage(storage) ;
  return mapping ;
  }



//      self._reverse = { str(m[0]): (k, m[1], m[3]) for k, m in self._mapping.iteritems() }
//prop: label, otype, rmap
 

dict *map_get_attributes(librdf_model *model, dict *map, const char *uri, const char *type)
/*=======================================================================================*/
{
  char *sparql ;
  asprintf(&sparql, "CONSTRUCT { <%s> ?p ?o } WHERE { <%s> a <%s> . <%s> ?p ?o }",
                                    uri,                 uri,  type,   uri) ;
  librdf_query *query = librdf_new_query(world, "sparql11-query", NULL,
                                                (unsigned char *)sparql, NULL) ;
  librdf_query_results *results = librdf_model_query_execute(model, query) ;

  if (results == NULL) { //...
    fprintf(stderr, "SPARQL error...\n") ;
    }
  else {

    if (librdf_query_results_is_graph(results)) {

      librdf_stream *stream = librdf_query_results_as_stream(results) ;
    
      while (!librdf_stream_end(stream)) {
        librdf_statement *stmt = librdf_stream_get_object(stream) ;

        librdf_node *pred = librdf_statement_get_predicate(stmt) ;
        Value *rmap = dict_get_element(reverse, librdf_uri_as_string(librdf_node_get_uri(pred))) ;

        // attr = rmap[0]
        librdf_node *object = librdf_statement_get_object(stmt) ;
        
        dict_attributes

        librdf_stream_next(stream) ;
        }
      librdf_free_stream(stream) ;
      }
    librdf_free_query_results(results) ;
    }

  librdf_free_query(query) ;
  free(sparql) ;


static void add_value(dict *d, char *key, librdf_node *node, dtype, rmapfn)
/*=======================================================================*/
{
  if (node == NULL) dict_delete(d, key) ;
  else {

    char *text = NULL ;


    if (librdf_node_is_resource(node))
      asprintf(&text, "<%s>", librdf_uri_as_string(librdf_node_get_uri(pred))) ;

    else if (librdf_node_is_blank(node))
      asprintf(&text, "_:%s", librdf_node_get_blank_identifier(node))

    else {    // Literal
      char *v = (char *)librdf_node_get_literal_value(node) ;

      librdf_uri *dt = librdf_node_get_literal_value_datatype_uri(node) ;
      if (dt) {
        char *dtype = NULL ;
        asprintf(&dtype, librdf_uri_as_string(dt)) ;
        switch (value_get_integer(dict_get_element(datatypes, dtype))) {

          use appropriate cast/atoi() and dict_set()

          }

      if dtype: v = _datatypes.get(dtype, str)(v) 

      if (map->reverse) then call
      mapping_function(name, param_string)



      eval(rmapfn)(v) if rmapfn else v



    for stmt in statements:
      m = self._reverse.get(str(statement.predicate.uri), (None, None, None))
      s, attr, v = (statement.subject.uri, m[0], self._makevalue(statement.object, m[1], m[2]))


      dict[attr] = v

  }


#ifdef UNITTEST

void map_print(MapEntry *m)
/*=======================*/
{
  if (m->property) printf("<%s>, ", librdf_uri_as_string(m->property)) ;
  else printf("NULL, ") ;
  if (m->class) printf("<%s>, ", librdf_uri_as_string(m->class)) ;
  else printf("NULL, ") ;
  if (m->type) printf("<%s>, ", librdf_uri_as_string(m->type)) ;
  else printf("NULL, ") ;
  if (m->mapped) printf("'%s', ", m->mapped) ;
  else printf("NULL, ") ;
  if (m->reverse) printf("'%s'", m->reverse) ;
  else printf("NULL") ;
  }

void print(const char *k, Value *v, void *p)
/*========================================*/
{
  printf("%s: (", k) ;
  map_print(v->pointer) ;
  printf(")\n") ;
  }



int main(void)
/*==========*/
{
  world = librdf_new_world() ;
  librdf_world_open(world) ;


  dict *maps = load_mapping(BSML_MAP_URI) ;

  dict_iterate(maps, print, NULL) ;

  dict_free(maps) ;

  librdf_free_world(world) ;
  }

#endif
