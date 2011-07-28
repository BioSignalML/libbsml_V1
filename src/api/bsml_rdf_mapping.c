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

#include "bsml_names.h"
#include "bsml_rdfnames.h"
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

static dict *datatypes    = NULL ;
static dict *pointerkinds = NULL ;
static dict *bsml_mapping = NULL ;

enum {
  KIND_NONE     =   0,
  KIND_DATETIME =  10,
  KIND_URI      = 100,
  KIND_NODE
  } ;

typedef struct {
  int sign ;
  struct tm datetime ;
  unsigned int usecs ;
  } DateTime ;



DateTime *DateTime_create(void)
/*===========================*/
{
  return (DateTime *)calloc(sizeof(DateTime), 1) ;
  }


void DateTime_free(DateTime *dt)
/*============================*/
{
  free(dt) ;
  }


void dict_set_datetime(dict *d, const char *key, DateTime *dt)
/*==========================================================*/
{
  dict_set_pointer(d, key, (void *)dt, KIND_DATETIME, (Free_Function *)DateTime_free) ;
  }

DateTime *dict_get_datetime(dict *d, const char *key)
/*=================================================*/
{
  int kind ;
  DateTime *dt = (DateTime *)dict_get_pointer(d, key, &kind) ;
  if (dt && kind == KIND_DATETIME) return dt ;
  return NULL ;
  }


void dict_set_uri(dict *d, const char *key, librdf_uri *uri)
/*========================================================*/
{
  dict_set_pointer(d, key, (void *)uri, KIND_URI, (Free_Function *)librdf_free_uri) ;
  }

librdf_uri *dict_get_uri(dict *d, const char *key)
/*==============================================*/
{
  int kind ;
  librdf_uri *uri = (librdf_uri *)dict_get_pointer(d, key, &kind) ;
  if (uri && kind == KIND_URI) return uri ;
  return NULL ;
  }


void dict_set_node(dict *d, const char *key, librdf_node *node)
/*===========================================================*/
{
  dict_set_pointer(d, key, (void *)node, KIND_NODE, (Free_Function *)librdf_free_node) ;
  }

librdf_node *dict_get_node(dict *d, const char *key)
/*================================================*/
{
  int kind ;
  librdf_node *node = (librdf_node *)dict_get_pointer(d, key, &kind) ;
  if (node && kind == KIND_NODE) return node ;
  return NULL ;
  }


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
  DateTime *dt = DateTime_create() ;
  dt->sign = 1 ;
  if (*str == '-') {
    ++str ;
    dt->sign = -1 ;
    }
  char *end = strptime(str, "%FT%T", &dt->datetime) ;

  while (end) {    // We break or return,,,
    if (*end == '.')
      dt->usecs = strtol(end+1, &end, 10) ;

    if (*end == 'Z') {
      ++end ;
      dt->datetime.tm_gmtoff = 0 ;
      }
    else if (*end == '+' || *end == '-') { // Timezone offset
      int sign = (*end == '+') ? 1 : -1 ;
      int hh = (int)strtol(end, &end, 10) ;
      if (*end != ':') break ;
      int mm = (int)strtol(end, &end, 10) ;
      if (hh <= 13 && mm < 60 || hh == 14 && mm == 0)
        dt->datetime.tm_gmtoff = sign*60L*(60*hh + mm) ;
      else
        break ;
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
  while (1) {      // We break on error else return
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
      days = (int)strtol(d, NULL, 10) ;
      if (*++s != 'T') break ;  // Could have two Ds
      }
    d = ++s ;
    while (isdigit(*s)) ++s ;
    if (*s == 'H') {
      hours = (int)strtol(d, NULL, 10) ;
      if (hours > 24) break ;
      d = ++s ;
      while (isdigit(*s)) ++s ;
      }
    if (*s == 'M') {
      mins = (int)strtol(d, NULL, 10) ;
      if (mins >= 60 || hours == 24 && mins != 0) break ;
      d = ++s ;
      while (isdigit(*s)) ++s ;
      }
    if (*s == 'S' || *s == '.') {
      secs = (int)strtol(d, NULL, 10) ;
      if (secs >= 60) break ;
      if (*s == '.') {
        d = ++s ;
        while (isdigit(*s)) ++s ;
        if (*s != 'S') break ;
        usecs = (int)strtol(d, NULL, 10) ;
        }
      d = ++s ;
      }
    if (*d) break ;  // Should be at end

    return 86400.0*(double)days + 3600.0*(double)hours + 60.0*(double)mins
           + (double)secs + (double)usecs/1000000.0 ;
    }
  return 0.0 ;
  }


const char *call_map_function_datetime(const char *fn, DateTime *dt)
/*================================================================*/
{
  return (strcmp(fn , "datetime_to_isoformat") == 0) ? datetime_to_isoformat(dt)
                                                     : NULL ;
  }

DateTime *inverse_map_function_datetime(const char *fn, const char *s)
/*==================================================================*/
{
  return (strcmp(fn , "isoformat_to_datetime") == 0) ? isoformat_to_datetime(s)
                                                     : NULL ;
  }


const char *call_map_function_string(const char *fn, const char *s)
/*===============================================================*/
{
  return s ;
  }

const char *inverse_map_function_string(const char *fn, const char *s)
/*==================================================================*/
{
  return s ;
  }


const char *call_map_function_long(const char *fn, long l, char *buf)
/*=================================================================*/
{
  sprintf(buf, "%d", l) ;
  return (const char *)buf ;
  }

long inverse_map_function_long(const char *fn, const char *s)
/*=========================================================*/
{
  return strtol(s, NULL, 10) ;
  }


const char *call_map_function_double(const char *fn, double d, char *buf)
/*=====================================================================*/
{
  if (strcmp(fn , "seconds_to_isoduration") == 0) return seconds_to_isoduration(d) ;
  sprintf(buf, "%g", d) ;
  return (const char *)buf ;
  }

double inverse_map_function_double(const char *fn, const char *s)
/*=============================================================*/
{
  return (strcmp(fn , "isoduration_to_seconds") == 0) ? isoduration_to_seconds(s)
                                                      : strtod(s, NULL) ;
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
  const char *label ;
  const char *class ;
  librdf_node *property ;
  librdf_uri *datatype ;
  const char *mapfn ;
  const char *inverse ;
  } MapEntry  ;     // Indexed by (label + class)


typedef struct {
  const char *label ;
  const char *datatype ;
  const char *mapfn ;
  } ReverseEntry ;  // Indexed (str(property) + class)


static librdf_node *results_get_node(librdf_query_results *r, const char *k)
/*========================================================================*/
{
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_resource(n)) return n ;
    librdf_free_node(n) ;
    }
  return NULL ;
  }

static librdf_uri *results_get_uri(librdf_query_results *r, const char *k)
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

static const char *results_get_string(librdf_query_results *r, const char *k)
/*=========================================================================*/
{
  const char *s = NULL ;
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, k) ;
  if (n) {
    if (librdf_node_is_literal(n))
      s = (const char *)librdf_node_get_literal_value(n) ;
    librdf_free_node(n) ;
    }
  return s ;
  }

static void map_entry_free(void *p)
/*===============================*/
{
  MapEntry *m = p ;
  if (m->label) free((void *)m->label) ;
  if (m->class) free((void *)m->class) ;
  if (m->property) librdf_free_node(m->property) ;
  if (m->datatype) librdf_free_uri(m->datatype) ;
  if (m->mapfn) free((void *)m->mapfn) ;
  if (m->inverse) free((void *)m->inverse) ;
  free(m) ;
  }

static void mapping_set(dict *map, librdf_query_results *r)
/*=======================================================*/
{
  MapEntry *m = (MapEntry *)calloc(sizeof(MapEntry), 1) ;
  const char *label = results_get_string(r, "lbl") ;
  const char *class = results_get_string(r, "class") ;
  if (label) {
    char key[strlen(label) + (class ? strlen(class) : 0) + 1] ;
    strcpy(key, label) ;
    if (class) strcat(key, class) ;
    m->label    = string_copy(label) ;
    m->class    = string_copy(class) ;
    m->property = results_get_node(r, "prop") ;
    m->datatype = results_get_uri(r, "otype") ;
    m->mapfn    = results_get_string(r, "map") ;
    m->inverse  = results_get_string(r, "rmap") ;
    dict_set_pointer(map, key, m, KIND_NONE, map_entry_free) ;
    }
  }


static int uri_protocol(const char *u)
/*==================================*/
{
  return (memcmp(u, "file:", 5) == 0 || memcmp(u, "http:", 5) == 0) ;
  }


static int load_mapping(dict *mapping, const char *mapfile)
/*=======================================================*/
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
    fprintf(stderr, "Failed to parse: %s\n", mapfile) ;
    return -1 ;
    }
  librdf_free_parser(parser);
  librdf_free_uri(uri) ;

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
  return 0 ;
  }


/****
//      self._inverse = { str(m[0]): (k, m[1], m[3]) for k, m in self._mapping.iteritems() }
//prop: label, otype, rmap


// mapping_new can specify more mapping files

  dict *mapping = dict_deep_copy(bsml_mapping) ;

  dict mymap = load_mapping(mapping, myfile) ;
  dict_update(mapping, mymap) ;
  dict_free(mymap) ;

  dict *inverse = ...


//

**/

typedef struct {
  librdf_model *model ;
  librdf_node *subject ;
  const char *class ;
  dict *mapping ;
  } MapSaveInfo ;


static void add_statement(const char *label, Value *value, void *userdata)
/*======================================================================*/
{
  MapSaveInfo *ud = (MapSaveInfo *)userdata ;
  VALUE_TYPE vtype = value_type(value) ;

  MapEntry *m = NULL ;
  if (ud->class) {
    char key[strlen(label) + strlen(ud->class) + 1] ;
    strcpy(key, label) ;
    strcat(key, ud->class) ;
    m = dict_get_pointer(ud->mapping, key, NULL) ;
    }
  if (m == NULL) m = dict_get_pointer(ud->mapping, label, NULL) ;

  if (m) {
    const char *literal = (vtype == TYPE_STRING) ? value_get_string(value) : NULL ;
    char numbuf[20] ;

    if (literal == NULL || *literal) {  // Not a string or a non-empty string
      librdf_node *node = NULL ;

      switch (vtype) {
       case TYPE_POINTER:
        { int kind ;
          void *p = value_get_pointer(value, &kind) ;
          if (p == NULL) break ;
          if (kind == KIND_NODE) {
            librdf_model_add(ud->model, ud->subject, m->property, (librdf_node *)p) ;
            return ;
            }
          else if (kind == KIND_URI)
            node = librdf_new_node_from_uri(world, (librdf_uri *)p) ;
          else if (kind == KIND_DATETIME)
            literal = call_map_function_datetime(m->mapfn, (DateTime *)p) ;
          }
        break ;

       case TYPE_STRING:
        if (uri_protocol(literal))
          node = librdf_new_node_from_uri_string(world, (unsigned char *)literal) ;
        else
          literal = call_map_function_string(m->mapfn, literal) ;
        break ;

       case TYPE_INTEGER:
        literal = call_map_function_long(m->mapfn, value_get_integer(value), numbuf) ;
        break ;

       case TYPE_REAL:
        literal = call_map_function_double(m->mapfn, value_get_real(value), numbuf) ;
        break ;

       default:
        break ;
        }

      if (literal && node == NULL) {
        node = librdf_new_node_from_typed_literal(world, (unsigned char *)literal, NULL, m->datatype) ;
        }
      if (node) {
        librdf_model_add(ud->model, ud->subject, m->property, node) ;
        librdf_free_node(node) ;
        }
      }

    }

  }


typedef struct {
  dict *mapping ;
  dict *reversemap ;
  } Mapping ;


/**
 Add attributes of a class instance with uri of 'subject' to a model
**/

void map_save_attributes(Mapping *map, librdf_model *model, const char *subject,
                                                            const char *class, dict *attributes)
/*============================================================================================*/
{
  if (subject == NULL)
    subject = dict_get_string(attributes, "uri") ;
  if (subject) {
    MapSaveInfo info ;
    info.model = model ;
    info.subject = librdf_new_node_from_uri_string(world, (unsigned char *)subject) ;
    info.class = class ;
    info.mapping = map->mapping ;
    dict_iterate(attributes, (Iterator_Function *)add_statement, &info) ;
    librdf_free_node(info.subject) ;
    }
  }

/*
  def statement(self, s, attr, v):
  #-------------------------------
    m = self._mapping[attr]
    return (s, m[0], self._makenode(v, m[1], m[2]))
*/



static void set_attribute(dict *attributes, librdf_node *node, ReverseEntry *rmap)
/*==============================================================================*/
{
  const char *key = rmap->label ;

  if (node == NULL)
    dict_delete(attributes, key) ;

  else {

/*
typedef struct {
  const char *label ;
  const char *datatype ;
  const char *mapfn ;
  } ReverseEntry ;  // Indexed (str(property) + class)
*/
    if (librdf_node_is_resource(node))
      dict_set_node(attributes, key, librdf_new_node_from_node(node)) ;

    else if (librdf_node_is_literal(node)) {
      char *text = (char *)librdf_node_get_literal_value(node) ;
      VALUE_TYPE dtype = 0 ;
      if (rmap->datatype)
        dtype = dict_get_integer(datatypes, rmap->datatype) ;
      if (dtype == 0) dtype = TYPE_STRING ;

      switch (dtype) {
       case TYPE_POINTER:
        { int kind = dict_get_integer(pointerkinds, rmap->datatype) ;
          if (kind == KIND_DATETIME)
            dict_set_datetime(attributes, key, inverse_map_function_datetime(rmap->mapfn, text)) ;
          }
        break ;

       case TYPE_STRING:
        dict_set_string(attributes, key, inverse_map_function_string(rmap->mapfn, text)) ;
        break ;

       case TYPE_INTEGER:
        dict_set_integer(attributes, key, inverse_map_function_long(rmap->mapfn, text)) ;
        break ;

       case TYPE_REAL:
        dict_set_real(attributes, key, inverse_map_function_double(rmap->mapfn, text)) ;
        break ;

       default:
        break ;
        }
      }
    }
  }



/**
  Load attributes of class with uri of 'subject' from a model.
**/

dict *map_get_attributes(Mapping *map, librdf_model *model, const char *subject,
                                                            const char *class)
/*=============================================================================*/
{
  char *sparql ;
  asprintf(&sparql, "CONSTRUCT { <%s> ?p ?o } WHERE { <%s> a <%s> . <%s> ?p ?o }",
                                                    subject, subject, class, subject) ;
  librdf_query *query = librdf_new_query(world, "sparql11-query", NULL,
                                                (unsigned char *)sparql, NULL) ;
  librdf_query_results *results = librdf_model_query_execute(model, query) ;
  free(sparql) ;
  if (results == NULL) fprintf(stderr, "SPARQL error...\n") ;

  dict *attributes = dict_create() ;
  if (results && librdf_query_results_is_graph(results)) {
    librdf_stream *stream = librdf_query_results_as_stream(results) ;
    while (!librdf_stream_end(stream)) {
      librdf_statement *stmt = librdf_stream_get_object(stream) ;
      ReverseEntry *rmap = NULL ;
      const char *pred = (char *)librdf_uri_as_string(
                           librdf_node_get_uri(
                             librdf_statement_get_predicate(stmt))) ;
      if (class) {
        char key[strlen(pred) + strlen(class) + 1] ;
        strcpy(key, pred) ;
        strcat(key, class) ;
        rmap = dict_get_pointer(map->reversemap, key, NULL) ;
        }
      if (rmap == NULL) rmap = dict_get_pointer(map->reversemap, pred, NULL) ;
      if (rmap) set_attribute(attributes, librdf_statement_get_object(stmt), rmap) ;
      librdf_stream_next(stream) ;
      }
    librdf_free_stream(stream) ;
    }
  librdf_free_query_results(results) ;
  librdf_free_query(query) ;

  return attributes ;
  }



void bsml_rdf_mapping_initialise(void)
/*==================================*/
{
  dict_set_integer(datatypes, XSD_float,              TYPE_REAL) ;
  dict_set_integer(datatypes, XSD_double,             TYPE_REAL) ;
  dict_set_integer(datatypes, XSD_integer,            TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_long,               TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_int,                TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_short,              TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_byte,               TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_nonPostiveInteger,  TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_nonNegativeInteger, TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_positiveInteger,    TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_negativeInteger,    TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedLong,       TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedInt,        TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedShort,      TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedByte,       TYPE_INTEGER) ;

  dict_set_integer(datatypes,    XSD_duration,        TYPE_REAL) ;
  dict_set_integer(datatypes,    XSD_dateTime,        TYPE_POINTER) ;
  dict_set_integer(pointerkinds, XSD_dateTime,        KIND_DATETIME) ;
  }




#ifdef UNITTEST

void map_print(MapEntry *m)
/*=======================*/
{
  if (m->label) printf("'%s', ", m->label) ;
  else printf("NULL, ") ;
  if (m->class) printf("'%s', ", m->class) ;
  else printf("NULL, ") ;
  if (m->property) printf("<%s>, ", librdf_uri_as_string(librdf_node_get_uri(m->property))) ;
  else printf("NULL, ") ;
  if (m->datatype) printf("<%s>, ", librdf_uri_as_string(m->datatype)) ;
  else printf("NULL, ") ;
  if (m->mapfn) printf("'%s', ", m->mapfn) ;
  else printf("NULL, ") ;
  if (m->inverse) printf("'%s'", m->inverse) ;
  else printf("NULL") ;
  }


void print(const char *k, Value *v, void *p)
/*========================================*/
{
  printf("%s: (", k) ;
  map_print(value_get_pointer(v, NULL)) ;
  printf(")\n") ;
  }


/*
eg. 'description' --> dc.description for Recording
    'description' --> rdfs.comment   for Event

ie. dc.description --> 'description' for Recording
    dc.description --> 'annotation' for Event
*/


int main(void)
/*==========*/
{
  world = librdf_new_world() ;
  librdf_world_open(world) ;

  bsml_rdf_mapping_initialise() ;

  dict *maps = dict_create() ;

  load_mapping(maps, BSML_MAP_URI) ;

  dict_iterate(maps, (Iterator_Function *)print, NULL) ;

  dict_free(maps) ;

  librdf_free_world(world) ;
  }

#endif
