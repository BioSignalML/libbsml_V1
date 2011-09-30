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
#include <ctype.h>

#include "bsml_rdf.h"
#include "bsml_rdf_mapping.h"
#include "bsml_internal.h"

/**

Role of mappings:

1) Take a dictionary of attribute/value pairs for some BSML resource
   and add them to a RDF model (or return a stream of RDF statements)

2) Given a RDF model and a uri of a BSML resource, return a dictionary
   of attribute/value pairs.

**/



char *BSML_MAP_URI = "file:///Users/dave/biosignalml/python/api/model/mapping.ttl" ;

// 'file://' + os.path.dirname(os.path.abspath(__file__)) + '/mapping.ttl'


extern librdf_world *world ;

static dict *datatypes    = NULL ;
static dict *pointerkinds = NULL ;
static dict *bsml_mapping = NULL ;



const char *datetime_to_isoformat(bsml_datetime *dt)
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

bsml_datetime *isoformat_to_datetime(const char *str)
/*============================================*/
{
  bsml_datetime *dt = bsml_datetime_create() ;
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


static const char *call_map_function_datetime(const char *fn, bsml_datetime *dt)
/*=======================================================================*/
{
  if (fn) {
    if (strcmp(fn , "datetime_to_isoformat") == 0) return datetime_to_isoformat(dt) ;
    }
  return NULL ;
  }

static bsml_datetime *inverse_map_function_datetime(const char *fn, const char *s)
/*=========================================================================*/
{
  if (fn) {
    if (strcmp(fn , "isoformat_to_datetime") == 0) return isoformat_to_datetime(s) ;
    }
  return NULL ;
  }


static const char *call_map_function_string(const char *fn, const char *s)
/*======================================================================*/
{
  return s ;
  }

static const char *inverse_map_function_string(const char *fn, const char *s)
/*=========================================================================*/
{
  return s ;
  }


static const char *call_map_function_long(const char *fn, long l, char *buf)
/*========================================================================*/
{
  sprintf(buf, "%d", l) ;
  return (const char *)buf ;
  }

static long inverse_map_function_long(const char *fn, const char *s)
/*================================================================*/
{
  return strtol(s, NULL, 10) ;
  }


static const char *call_map_function_double(const char *fn, double d, char *buf)
/*============================================================================*/
{
  if (fn) {
    if (strcmp(fn , "seconds_to_isoduration") == 0) return seconds_to_isoduration(d) ;
    }
  sprintf(buf, "%g", d) ;
  return (const char *)buf ;
  }

static double inverse_map_function_double(const char *fn, const char *s)
/*====================================================================*/
{
  if (fn) {
    if (strcmp(fn , "isoduration_to_seconds") == 0) return isoduration_to_seconds(s) ;
    }
  return strtod(s, NULL) ;
  }



static char *QUERY_MAP =
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


struct bsml_rdfmapping {
  dict *mapping ;
  dict *reversemap ;
  } ;



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

static void mapping_set(librdf_query_results *r, void *userdata)
//==============================================================
{
  dict *map = (dict *)userdata ;
  MapEntry *m = (MapEntry *)calloc(sizeof(MapEntry), 1) ;
  const char *label = results_get_string(r, "lbl") ;
  const char *class = results_get_string(r, "class") ;
  if (label) {
    char key[strlen(label) + (class ? strlen(class) : 0) + 1] ;
    strcpy(key, label) ;
    if (class) strcat(key, class) ;
    m->label    = label ;
    m->class    = class ;
    m->property = results_get_node(r, "prop") ;
    m->datatype = results_get_uri(r, "otype") ;
    m->mapfn    = results_get_string(r, "map") ;
    m->inverse  = results_get_string(r, "rmap") ;
    dict_set_pointer(map, key, m, BSML_KIND_NONE, map_entry_free) ;
    }
  }


static void load_mapping(dict *mapping, const char *mapfile)
/*========================================================*/
{
  bsml_graphstore *g = bsml_graphstore_create_from_uri(mapfile, "turtle") ;
  if (g) {
    bsml_graphstore_select(g, QUERY_MAP, mapping_set, (void *)mapping) ;
    bsml_graphstore_free(g) ;
    }
  }


static void mapping_copy(const char *k, Value *value, void *userdata)
//===================================================================
{
  dict *copy = (dict *)userdata ;
  MapEntry *m = value_get_pointer(value, NULL) ;
  if (m->property) {
    MapEntry *n = (MapEntry *)calloc(sizeof(MapEntry), 1) ;
    n->property = librdf_new_node_from_node(m->property) ;
    if (m->label)    n->label    = string_copy(m->label) ;
    if (m->class)    n->class    = string_copy(m->class) ;
    if (m->datatype) n->datatype = librdf_new_uri_from_uri(m->datatype) ;
    if (m->mapfn)    n->mapfn    = string_copy(m->mapfn) ;
    if (m->inverse)  n->inverse  = string_copy(m->inverse) ;
    dict_set_pointer(copy, k, n, BSML_KIND_NONE, map_entry_free) ;
    }
  }


static void reverse_entry_free(void *p)
/*===================================*/
{
  ReverseEntry *m = p ;
  if (m->label) free((void *)m->label) ;
  if (m->datatype) free((void *)m->datatype) ;
  if (m->mapfn) free((void *)m->mapfn) ;
  free(m) ;
  }

static void reverse_set(const char *k, Value *value, void *userdata)
/*================================================================*/
{
  dict *rmap = (dict *)userdata ;
  MapEntry *m = value_get_pointer(value, NULL) ;
  if (m->property) {
    ReverseEntry *r = (ReverseEntry *)calloc(sizeof(ReverseEntry), 1) ;

    char *prop = (char *)librdf_uri_as_string(librdf_node_get_uri(m->property)) ;
    char key[strlen(prop) + (m->class ? strlen(m->class) : 0) + 1] ;
    strcpy(key, prop) ;
    if (m->class) strcat(key, m->class) ;

    if (m->label)    r->label    = string_copy(m->label) ;
    if (m->datatype) r->datatype = string_copy((char *)librdf_uri_as_string(m->datatype)) ;
    if (m->inverse)  r->mapfn    = string_copy(m->inverse) ;

    dict_set_pointer(rmap, key, r, BSML_KIND_NONE, reverse_entry_free) ;
    }
  }


bsml_rdfmapping *bsml_rdfmapping_create(dict *usermap)
/*==================================*/
{
  bsml_rdfmapping *m = (bsml_rdfmapping *)calloc(sizeof(bsml_rdfmapping) , 1) ;

  m->mapping = dict_create() ;
  dict_iterate(bsml_mapping, mapping_copy, (void *)(m->mapping)) ;
  if (usermap) dict_iterate(usermap, mapping_copy, (void *)(m->mapping)) ;

  m->reversemap = dict_create() ;
  dict_iterate(m->mapping, reverse_set, (void *)(m->reversemap)) ;

  return m ;
  }

void bsml_rdfmapping_free(bsml_rdfmapping *m)
/*=========================*/
{
  dict_free(m->reversemap) ;
  dict_free(m->mapping) ;
  free(m) ;
  }

/****

// mapping_new can specify more mapping files

  dict *mapping = dict_deep_copy(bsml_mapping) ;

  dict mymap = load_mapping(mapping, myfile) ;
  dict_update(mapping, mymap) ;
  dict_free(mymap) ;

  dict *inverse = ...

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
    m = (MapEntry *)dict_get_pointer(ud->mapping, key, NULL) ;
    }
  if (m == NULL) m = (MapEntry *)dict_get_pointer(ud->mapping, label, NULL) ;

  if (m) {
    const char *literal = (vtype == VALUE_TYPE_STRING) ? value_get_string(value) : NULL ;
    char numbuf[20] ;

    if (literal == NULL || *literal) {  // Not a string or a non-empty string
      librdf_node *node = NULL ;

      switch (vtype) {
       case VALUE_TYPE_POINTER:
        { int kind ;
          void *p = value_get_pointer(value, &kind) ;
          if (p == NULL) break ;
          if (kind == BSML_KIND_NODE) {
            librdf_model_add(ud->model,
              librdf_new_node_from_node(ud->subject),
              librdf_new_node_from_node(m->property),
              librdf_new_node_from_node((librdf_node *)p)) ;
            return ;
            }
          else if (kind == BSML_KIND_URI)
            node = librdf_new_node_from_uri(world, (librdf_uri *)p) ;
          else if (kind == BSML_KIND_DATETIME)
            literal = call_map_function_datetime(m->mapfn, (bsml_datetime *)p) ;
          }
        break ;

       case VALUE_TYPE_STRING:
        if (bsml_uri_protocol(literal))
          node = librdf_new_node_from_uri_string(world, (unsigned char *)literal) ;
        else
          literal = call_map_function_string(m->mapfn, literal) ;
        break ;

       case VALUE_TYPE_INTEGER:
        literal = call_map_function_long(m->mapfn, value_get_integer(value), numbuf) ;
        break ;

       case VALUE_TYPE_REAL:
        literal = call_map_function_double(m->mapfn, value_get_real(value), numbuf) ;
        break ;

       default:
        break ;
        }

      if (literal && node == NULL) {
        node = librdf_new_node_from_typed_literal(world, (unsigned char *)literal, NULL, m->datatype) ;
        }
      if (node) {
        librdf_model_add(ud->model,
          librdf_new_node_from_node(ud->subject),
          librdf_new_node_from_node(m->property),
          node) ;
        }
      }

    }
  }


/**
 Add attributes of a class instance with uri of 'subject' to a model
**/

void bsml_rdfmapping_save_attributes(bsml_rdfmapping *map, bsml_graphstore *g, dict *attributes,
                                                          const char *subject, const char *class)
/*===============================================================================================*/
{
  if (subject == NULL)
    subject = dict_get_string(attributes, "uri") ;
  if (subject) {
    MapSaveInfo info ;
    info.model = g->model ;
    info.subject = librdf_new_node_from_uri_string(world, (unsigned char *)subject) ;
    info.class = class ;
    info.mapping = map->mapping ;
    dict_iterate(attributes, add_statement, (void *)&info) ;
    librdf_free_node(info.subject) ;
    }
  }

/*
  def statement(self, s, attr, v):
  #-------------------------------
    m = self._mapping[attr]
    return (s, m[0], self._makenode(v, m[1], m[2]))
*/



static void set_attribute(dict *attributes, librdf_node *node, ReverseEntry *r)
/*===========================================================================*/
{
  const char *key = r->label ;

  if (node == NULL)
    dict_delete(attributes, key) ;

  else {

    if (librdf_node_is_resource(node))
      dict_set_node(attributes, key, librdf_new_node_from_node(node)) ;

    else if (librdf_node_is_literal(node)) {
      char *text = (char *)librdf_node_get_literal_value(node) ;
      VALUE_TYPE dtype = 0 ;
      if (r->datatype)
        dtype = dict_get_integer(datatypes, r->datatype) ;
      if (dtype == 0) dtype = VALUE_TYPE_STRING ;

      switch (dtype) {
       case VALUE_TYPE_POINTER:
        { int kind = dict_get_integer(pointerkinds, r->datatype) ;
          if (kind == BSML_KIND_DATETIME)
            dict_set_datetime(attributes, key, inverse_map_function_datetime(r->mapfn, text)) ;
          }
        break ;

       case VALUE_TYPE_STRING:
        dict_set_string(attributes, key, inverse_map_function_string(r->mapfn, text)) ;
        break ;

       case VALUE_TYPE_INTEGER:
        dict_set_integer(attributes, key, inverse_map_function_long(r->mapfn, text)) ;
        break ;

       case VALUE_TYPE_REAL:
        dict_set_real(attributes, key, inverse_map_function_double(r->mapfn, text)) ;
        break ;

       default:
        break ;
        }
      }
    }
  }



typedef struct {
  bsml_rdfmapping *map ;
  const char *class ;
  dict *attributes ;
  } MapGetInfo ;



static void map_statements(librdf_statement *stmt, void *userdata)
//================================================================
{
  MapGetInfo *info = userdata ;

  ReverseEntry *r = NULL ;
  const char *pred = (char *)librdf_uri_as_string(
                       librdf_node_get_uri(
                         librdf_statement_get_predicate(stmt))) ;
  if (info->class) {
    char key[strlen(pred) + strlen(info->class) + 1] ;
    strcpy(key, pred) ;
    strcat(key, info->class) ;
    r = (ReverseEntry *)dict_get_pointer(info->map->reversemap, key, NULL) ;
    }
  if (r == NULL) r = (ReverseEntry *)dict_get_pointer(info->map->reversemap, pred, NULL) ;
  if (r) set_attribute(info->attributes, librdf_statement_get_object(stmt), r) ;
  }



/**
  Load attributes of class with uri of 'subject' from a model.
**/

void bsml_rdfmapping_get_attributes(bsml_rdfmapping *map, dict *attributes, bsml_graphstore *g,
                                                        const char *subject, const char *class)
/*===========================================================================================*/
{
  dict_set_uri(attributes, "uri", librdf_new_uri(world, (const unsigned char *)subject)) ;
  MapGetInfo info = { map, class, attributes } ;
  char *sparql ;
  asprintf(&sparql, "CONSTRUCT { <%s> ?p ?o } WHERE { <%s> a <%s> . <%s> ?p ?o }",
                                                    subject, subject, class, subject) ;
  bsml_graphstore_construct(g, sparql, map_statements, (void *)&info) ;
  free(sparql) ;
  }


void bsml_rdf_mapping_initialise(void)
/*==================================*/
{
  datatypes = dict_create() ;
  pointerkinds = dict_create() ;

  dict_set_integer(datatypes, XSD_float,              VALUE_TYPE_REAL) ;
  dict_set_integer(datatypes, XSD_double,             VALUE_TYPE_REAL) ;
  dict_set_integer(datatypes, XSD_integer,            VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_long,               VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_int,                VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_short,              VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_byte,               VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_nonPostiveInteger,  VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_nonNegativeInteger, VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_positiveInteger,    VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_negativeInteger,    VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedLong,       VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedInt,        VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedShort,      VALUE_TYPE_INTEGER) ;
  dict_set_integer(datatypes, XSD_unsignedByte,       VALUE_TYPE_INTEGER) ;

  dict_set_integer(datatypes,    XSD_duration,        VALUE_TYPE_REAL) ;
  dict_set_integer(datatypes,    XSD_dateTime,        VALUE_TYPE_POINTER) ;
  dict_set_integer(pointerkinds, XSD_dateTime,        BSML_KIND_DATETIME) ;


  bsml_mapping = dict_create() ;
  load_mapping(bsml_mapping, BSML_MAP_URI) ;
  }


void bsml_rdfmapping_finish(void)
//===============================
{
  if (bsml_mapping) dict_free(bsml_mapping) ;
  if (pointerkinds) dict_free(pointerkinds) ;
  if (datatypes)    dict_free(datatypes) ;
  }


#ifdef RDFMAPTEST

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


static void print(const char *k, Value *v, void *p)
/*===============================================*/
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



static void print_attr(const char *k, Value *v, void *p)
/*====================================================*/
{
  void *ptr ;
  int kind ;
  switch (value_type(v)) {
   case VALUE_TYPE_POINTER:
    ptr = value_get_pointer(v, &kind) ;
    if (kind == BSML_KIND_URI)
      printf("'%s': <%s>\n", k, librdf_uri_as_string((librdf_uri *)ptr)) ;
    else if (kind == BSML_KIND_NODE)
      printf("'%s': <%s>\n", k, librdf_uri_as_string(librdf_node_get_uri((librdf_node *)ptr))) ;
    else
      printf("'%s': 0x%08lx (%d)\n", k, ptr, kind) ;
    break ;
   case VALUE_TYPE_STRING:
    printf("'%s': '%s'\n", k, value_get_string(v)) ;
    break ;
   case VALUE_TYPE_INTEGER:
    printf("'%s': %d\n", k, value_get_integer(v)) ;
    break ;
   case VALUE_TYPE_REAL:
    printf("'%s': %g\n", k, value_get_real(v)) ;
    break ;
   default:
    break ;
    }
  }


void print_attributes(dict *a)
/*==========================*/
{
  dict_iterate(a, print_attr, NULL) ;
  }

dict *get_metadata(bsml_rdfmapping *map, bsml_graphstore *g, const char *uri, const char *class)
/*===============================================================================*/
{
  dict *md = dict_create() ;
  bsml_rdfmapping_get_attributes(map, md, g, uri, class) ;
  print_attributes(md) ;
  return md ;
  }



int main(void)
/*==========*/
{
  bsml_rdf_initialise() ;

  //dict_iterate(bsml_mapping, (Iterator_Function *)print, NULL) ;

  bsml_rdfmapping *mymap = bsml_rdfmapping_create(NULL) ;

  //dict_print(mymap->reversemap) ;

  char *rdf = "file:///Users/dave/biosigna^lml/libbsml/src/api/edf.ttl" ;
  bsml_graphstore *edfgraph = bsml_graphstore_create_from_uri(rdf, "turtle") ;
  dict *rec = get_metadata(mymap, edfgraph,
     "http://recordings.biosignalml.org/testdata/sinewave", BSML_Recording) ;
  dict *sig = get_metadata(mymap, edfgraph,
     "http://recordings.biosignalml.org/testdata/sinewave/signal/1", BSML_Signal) ;
  bsml_graphstore_free(edfgraph) ;

  bsml_graphstore *out = bsml_graphstore_create() ;
  bsml_rdfmapping_save_attributes(mymap, out, rec,
     "http://recordings.biosignalml.org/testdata/sinewave", BSML_Recording) ;
  bsml_rdfmapping_save_attributes(mymap, out, sig,
     "http://recordings.biosignalml.org/testdata/sinewave/signal/1", BSML_Signal) ;
  librdf_model_print(out->model, stdout) ;   // *******
  bsml_graphstore_free(out) ;

  dict_free(rec) ;
  dict_free(sig) ;

  bsml_rdfmapping_free(mymap) ;

  dict_free(bsml_mapping) ;
  librdf_free_world(world) ;
  }

#endif
