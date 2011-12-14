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
#include <sys/param.h>

#include "bsml_model.h"

#include "bsml_rdf.h"
#include "bsml_rdf_mapping.h"
#include "bsml_names.h"
#include "bsml_internal.h"


static bsml_rdfmapping *bsml_mapping ;

void bsml_model_initialise(const char *repo)
//==========================================
{
  bsml_stream_initialise() ;
  bsml_rdf_initialise(repo) ;
  bsml_mapping = bsml_rdfmapping_create(NULL) ;
  }

void bsml_model_finish(void)
//==========================
{
  bsml_rdfmapping_free(bsml_mapping) ;
  bsml_rdf_finish() ;
  bsml_stream_finish() ;
  }


static void add_recording(librdf_query_results *r, void *userdata)
//================================================================
{
  const char *uri = NULL ;
  librdf_node *n = librdf_query_results_get_binding_value_by_name(r, "r") ;
  if (librdf_node_is_resource(n)) uri = (char *)librdf_uri_as_string(librdf_node_get_uri(n)) ;
  librdf_free_node(n) ;
  if (uri) {
    bsml_recording *rec = bsml_recording_create(uri, NULL, NULL, BSML_RECORDING) ;
    list_append_pointer((list *)userdata, (void *)rec, BSML_KIND_RECORDING,
                                                       (Value_Free *)bsml_recording_free) ;
    }
  }


list *bsml_model_recordings(bsml_graph *g)
//========================================
{
  list *recs = list_create() ;
  char *sparql ;
  asprintf(&sparql, "SELECT ?r WHERE { ?r a <%s> }", BSML_Recording) ;
  bsml_graph_select(g, sparql, add_recording, (void *)recs) ;
  free(sparql) ;
  return recs ;
  }


// Abstract Recordings...

bsml_recording *bsml_recording_create(const char *uri, dict *attributes,
                                                       bsml_graph *graph, int type)
//=================================================================================
{
  bsml_recording *rec = (bsml_recording *)calloc(sizeof(bsml_recording), 1) ;

  rec->type = type ;           // Look up format if graph given...

  if (uri) rec->uri = string_copy(uri) ;

  if (attributes == NULL) {
    attributes = dict_create() ;
    if (uri && graph) {
      bsml_rdfmapping_get_attributes(bsml_mapping, attributes, graph, uri, BSML_Recording) ;
// set type from attributes["format"]
      }
    }
  rec->attributes = attributes ;
  
  if (graph == NULL) graph = bsml_graph_create() ;
  rec->graph = graph ;
  
  return rec ;
  }


bsml_recording *bsml_recording_open_uri(const char *endpoint, const char *uri)
//============================================================================
{
  if (uri == NULL) return NULL ;
  bsml_graph *g = bsml_graph_create_from_uri(endpoint, uri) ;
  bsml_recording *rec = bsml_recording_create(uri, NULL, g, -1) ;
  if (g->location) rec->location = string_copy(g->location) ;
//dict_print(rec->attributes) ;
  return rec ;
  }


void bsml_recording_free(bsml_recording *r)
//=========================================
{
  if (r->graph) bsml_graph_free(r->graph) ;
  if (r->attributes) dict_free(r->attributes) ;
  if (r->location) free((void *)r->location) ;
  if (r->uri) free((void *)r->uri) ;
  }

void *bsml_recording_add_signal(bsml_recording *r, bsml_signal *s)
//===============================================================
{
  return NULL ; // **********************
  }

dict *bsml_recording_get_metavars(bsml_recording *r)
//==================================================
{
  return dict_create() ;
  }


char *bsml_recording_metadata_as_string(bsml_recording *r, const char *format, dict *prefixes)
//============================================================================================
{
  return "" ;     // ******************
  }


// Abstract Signals...

bsml_signal *bsml_signal_create(const char *uri, dict *attributes)
//================================================================
{
  bsml_signal *s = (bsml_signal *)calloc(sizeof(bsml_signal), 1) ;
  if (uri) s->uri = string_copy(uri) ;
  if (attributes == NULL) attributes = dict_create() ;
  s->attributes = attributes ;
  s->data = bsml_timeseries_create(NULL) ;
  return s ;
  }

void bsml_signal_free(bsml_signal *s)
//===================================
{
  if (s->data) bsml_timeseries_free(s->data) ;
  if (s->attributes) dict_free(s->attributes) ;
  if (s->location) free((void *)s->location) ;
  if (s->uri) free((void *)s->uri) ;
  free((void *)s) ;
  }


bsml_signal *bsml_signal_open_uri(const char *endpoint, const char *uri)
//======================================================================
{
  bsml_graph *g = bsml_graph_create_from_uri(endpoint, uri) ;
  if (g == NULL) return NULL ;

  bsml_signal *sig = bsml_signal_create(uri, NULL) ; 
  bsml_rdfmapping_get_attributes(bsml_mapping, sig->attributes, g, uri, BSML_Signal) ;
  if (g->location) sig->location = string_copy(g->location) ;
  bsml_graph_free(g) ;

//dict_print(sig->attributes) ;
  sig->recording = bsml_recording_open_uri(endpoint, dict_get_string(sig->attributes, "recording")) ;

  sig->data->rate = dict_get_real(sig->attributes, "rate") ;
  if (sig->data->rate) sig->data->period = 1.0/sig->data->rate ;

  return sig ;
  }


dict *bsml_signal_get_metavars(bsml_signal *s)
//============================================
{
  return dict_create() ;
  }


// Recordings as files...

bsml_recording *bsml_file_recording_init(const char *fname, const char *uri, dict *attributes, int type)
//======================================================================================================
{
  if (attributes == NULL) attributes = dict_create() ;
  if (type == 0) type = BSML_RECORDING_RAW ;

  if (fname && *fname) {
    char source[PATH_MAX+7] = "file://" ;
    realpath(fname, source+7) ;
    dict_set_string(attributes, "source", source) ;
    if (!(uri && *uri)) uri = source ;
    }
  else
    dict_set_string(attributes, "source", uri) ;

  if (dict_get_value(attributes, "format", NULL) == NULL)
    dict_set_string(attributes, "format", BSML_RAW) ;

// RAW Rccording sets 'source' and 'digest' properties and sets uri from fname if no uri.

  bsml_recording *r = bsml_recording_create(uri, attributes, NULL, type) ;

  const char *digest = dict_get_string(attributes, "digest") ;
  if (digest) dict_set_string(r->attributes, "digest", digest) ;

  return r ;
  }

void bsml_file_recording_close(bsml_recording *r)
//===============================================
{
  bsml_recording_free(r) ;
  }


// Signals in files...

bsml_signal *bsml_file_signal_init(const char *uri, dict *attributes)
//===================================================================
{
  return bsml_signal_create(uri, attributes) ;
  }

void bsml_file_signal_close(bsml_signal *s)
//=========================================
{
  bsml_signal_free(s) ;
  }





/*******
typedef struct {
  char *uri ;
  char *description ;
  char *type ;  // RDF.Node
  char **metadata
  } bsml_Metadata ;



Model attributes are in struct fields??


  sig.description versus bsml_signal_get_description(sig) ;


  sig.rate

  Want r/only structure fields...


:signal_rate
  a map:Mapping ;
  map:class "Signal" ;
  rdfs:label "rate" ;
  map:object xsd:double ;
  map:property bsml:sampleRate
  .


metadata properties/attributes:

  name of property
  value



resource *r

  r.type == bsml:Class

    if r.fieldname == 'description'

      val = obj.description



Class hierarchy

  HDF5Recording RAWRecording Recording Metadata
  
  

typedef struct Resource {
  char *type ;
  dict *metadata ;
  struct Resource *parent ; 
  } Resource ;


typedef struct {
  char *uri ;
  Resource *self ;
  } Metadata ;


typedef struct {
  double rate ;
  char *units ;

  Resource *self ;
  } Signal ;



Resource *bsml_create_resource(const char *type, void *parent)
//============================================================
{
  Resource *r = (Resource *)calloc(sizeof(Resource), 1) ;
  
  //r->type = string_copy(type) ;
  r->metadata = dict_create() ;
  //r->parent = parent ;
  return r ;
  }


Signal *bsml_create_metadata(const char *uri)
//===========================================
{
  Metadata *m = (Metadata *)calloc(sizeof(Metadata), 1) ;
  m->uri = string_copy(uri) ;
  m->self = bsml_create_resource(uri, "Metadata", NULL) ) ;
  return m ;
  }


Signal *bsml_create_signal(const char *uri)
//=========================================
{
  Signal *s = (Signal *)calloc(sizeof(Signal), 1) ;
  s->self = bsml_create_resource("Signal", bsml_create_metadata(uri)) ;


  // Init s->self->metadata from RDF
  s.rate = 

  return s ;
  }


double bsml_signal_rate(Signal *sig)
//==================================
{
  char *s = dictionary_get(sig->self->metadata, "rate") ;
  double rate = 0.0 ;
  if (s) sscanf(s, "%f", &rate) ;
  return rate ;
  }


void bsml_signal_set_rate(Signal *sig, double rate)
//=================================================
{
  char buf[32] ;
  sprintf(buf, "%f", rate) ;
  dictionary_set(sig->self->metadata, "rate", buf) ;
  }

/***
'Signal': ({ 'rate': ('double', '%f'), 'units': 'char *' }, 'Metadata')




resource_get_value(Resource *r, const char *key)
{
  while (r) {
    char *v = dictionary_get(r->metadata, key) ;
    if (v) return v ;
    r = r->parent ;
    }
  return NULL ;
  }




  dictionary_set(d, "1", "a") ;
  dictionary_set(d, "2", "b") ;
  dictionary_set(d, "3", "c") ;
  dictionary_iterate(d, print) ;
  
  printf("Element '2' is %s\n", dictionary_get(d, "2")) ;

  if (dictionary_get(d, "XX")) printf("ERROR...!!\n") ;
  else printf("No element found, as expected\n") ;

  dictionary_set(d, "1", "d") ;
  dictionary_del(d, "3") ;
  dictionary_set(d, "23", "ZZZZZZZZZZZb") ;
  dictionary_iterate(d, print) ;

  dictionary_free(d) ;
  }



  def __init__(self, uri, rdftype, metadata={}):
  #---------------------------------------------
    self.uri = Uri(uri)
    self.type = rdftype
    self.metadata = { }
    self.set_metavars(metadata)

  def set_metavars(self, meta):
  #----------------------------
    for cls in self.__class__.__mro__:
      if 'attributes' in cls.__dict__:
        for attr in cls.__dict__['attributes']:
          if attr in meta: setattr(self, attr, meta.get(attr))

  def get_metavars(self):
  #----------------------
    metadata = { }
    for cls in self.__class__.__mro__:
      if 'attributes' in cls.__dict__:
        for attr in cls.__dict__['attributes']:
          value = getattr(self, attr, None)
          if value is not None: metadata[attr] = value
    return metadata

  def meta_attributes(self):
  #-------------------------
    keys = set()
    for cls in self.__class__.__mro__:
      if 'attributes' in cls.__dict__: keys.update(cls.__dict__['attributes'])
    return list(keys)

  def makeuri(self, sibling=False):
  #--------------------------------
    u = str(self.uri)
    if   u.endswith(('/', '#')): return '%s%s'  % (u, uuid.uuid1())
    elif sibling:
      slash = u.rfind('/')
      hash  = u.rfind('#')
      if hash > slash:           return '%s#%s' % (u.rsplit('#', 1)[0], uuid.uuid1())
      else:                      return '%s/%s' % (u.rsplit('/', 1)[0], uuid.uuid1())
    else:                        return '%s/%s' % (u, uuid.uuid1())

  def map_to_graph(self, graph, rdfmap):
  #-------------------------------------
    graph.append(Statement(self.uri, rdf.type, self.type))
    graph.add_metadata(self, rdfmap)

  def _assign(self, attr, value):
  #------------------------------
    if attr in self.__dict__: setattr(self, attr, value)
    else:                     self.metadata[attr] = value

  @classmethod
  def create_from_repository(cls, uri, repo, rdfmap, **kwds):
  #----------------------------------------------------------
    self = cls(uri, **kwds)
    statements = repo.statements('<%(uri)s> ?p ?o',
                                  '<%(uri)s> a  <%(type)s> . <%(uri)s> ?p ?o',
                                  { 'uri': str(uri), 'type': str(self.type) })
    for stmt in statements:
      s, attr, v = rdfmap.metadata(stmt)
      self._assign(attr, v)
    return self

  def set_from_graph(self, attr, graph, rdfmap):
  #---------------------------------------------
    v = rdfmap.get_value_from_graph(self.uri, attr, graph)
    if v: self._assign(attr, v)



typedef struct {

  bsml_Metadata ...  // uri, type/class

  char *label ;
  'source',
  'format',
  'comment',
  'investigation',
  'starttime',
  'duration',

  bsml_RelativeTimeLine *timeline ;

  _signals = { }
  _signal_uris = [ ]
  _events = { }

  } bsml_Recording ;






  def __init__(self, uri, metadata={}):
  #------------------------------------
    super(Recording, self).__init__(uri, BSML_Recording, metadata=metadata)
    self.timeline = RelativeTimeLine(str(uri) + '/timeline')
    self._signals = { }
    self._signal_uris = [ ]
    self._events = { }

  def signals_from_repository(self, repo, rdfmap):
  #-----------------------------------------------
    for sig in repo.get_subjects(BSML.recording, self.uri):
      self.add_signal(Signal.create_from_repository(sig, repo, rdfmap, recording=self))

  def signals(self):
  #-----------------
    return [ self._signals[s] for s in self._signal_uris ]

  def add_signal(self, signal):
  #----------------------------
    """Add a :class:`Signal` to a Recording.

    :param signal: The signal to add to the recording.
    :type signal: :class:`Signal`
    :return: The 1-origin index of the signal in the recording.
    """
    if signal.uri in self._signal_uris:
       raise Exception, "Signal '%s' already in recording" % signal.uri
    self._signal_uris.append(signal.uri)
    self._signals[signal.uri] = signal
    return len(self._signal_uris) - 1         # 0-origin index of newly added signal uri

  def get_signal(self, uri=None, index=0):
  #---------------------------------------
    """Retrieve a :class:`Signal` from a Recording.

    :param uri: The uri of the signal to get.
    :param index: The 1-origin index of the signal to get.
    :return: A signal in the recording.
    :rtype: :class:`Signal`
    """
    if uri is None: uri = self._signal_uris[index]
    return self._signals[uri]

  def __len__(self):
  #-----------------
    return len(self._signals)


  def events(self):
  #-----------------
    return self._events.itervalues()

  def add_event(self, event):
  #--------------------------
    self._events[event.uri] = event
    event.factor = self

  def get_event(self, uri):
  #------------------------
    return self._events[uri]


  def instant(self, when):
  #----------------------
    return self.timeline.instant(when)

  def interval(self, start, duration):
  #-----------------------------------
    return self.timeline.interval(start, duration)


  def map_to_graph(self, rdfmap=None):
  #-----------------------------------
    graph = Graph(self.uri)
    if rdfmap is None: rdfmap = bsml_mapping()
    Metadata.map_to_graph(self, graph, rdfmap)
    Metadata.map_to_graph(self.timeline, graph, rdfmap)
    for s in self.signals(): s.map_to_graph(graph, rdfmap)
    for e in self._events.itervalues(): e.map_to_graph(graph, rdfmap)
    return graph


  def metadata_as_string(self, format='turtle', prefixes={ }):
  #-----------------------------------------------------------
    namespaces = { 'bsml': BSML.uri }
    namespaces.update(NAMESPACES)
    namespaces.update(prefixes)
    return self.map_to_graph().serialise(base=self.uri, format=format, prefixes=namespaces)


**/


