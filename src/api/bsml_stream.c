/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

/**

  A single Recording per STREAM, either with separate datasets for
  each Signal of the Recording::


  or with a group of signals sharing common timing:

  We recognise a BSML STREAM because it:

  * Has a '\metadata' dataset of STRING type, with a 'format' attribute,
    and valid RDF (in the specified format) as the contents.
    
  * Has a top-level group having a 'uri' attribute, with the group's name being
    equal to the attributes value with '/' replaced by '_'.

  * The metadata dataset contains the statement "<uri-value> a bsml:Recording".


  We identify the file as having a group of signals if the recording group:

  * Has a 2-D numeric dataset called './data'.

  * Has a 1-D string dataset called './uris'.

**/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "blockio.h"

#include "bsml_rdf.h"
#include "bsml_rdf_mapping.h"

#include "bsml_stream.h"

typedef struct {
  const char *version ;
  char **uris ;
  int channels ;
  } STREAMRecInfo ;

typedef struct {
  int index ;
  } STREAMSigInfo ;


#define DATASET_META  "metadata"
#define DATASET_DATA  "data"
#define DATASET_URIS  "uris"


// Helper functions:

static char *normalise_name(const char *name)
//===========================================
{
  char *nm = (char *)string_copy(name) ;
  char *s = nm ;
  while (1) {
    s = strchr(s, '/') ;
    if (s) *s = '_' ;
    else break ;
    }
  return nm ;
  }

static char *make_name(const char *prefix, const char *uri)
//=========================================================
{
  if (prefix && *prefix && strstr(uri, prefix) == uri) {
    uri += strlen(prefix) ;
    if (*uri == '/') uri += 1 ;
    }
  return normalise_name(uri) ;
  }


static int array_len(char **array)
//================================
{
  int n = 0 ;
  while (*array++) ++n ;
  return n ;
  }


static herr_t set_stream_attribute(hid_t stream, const char *attr, Value *value)
//==============================================================================
{
  herr_t status = 0 ;

  switch (value_type(value)) {
   case VALUE_TYPE_INTEGER: {
    long val = value_get_integer(value) ;
    status = H5LTset_attribute_long(stream, ".", attr, &val, 1) ;
    break ;
    }
   case VALUE_TYPE_REAL: {
    double val = value_get_real(value) ;
    status = H5LTset_attribute_double(stream, ".", attr, &val, 1) ;
    break ;
    }
   case VALUE_TYPE_STRING:
    status = H5LTset_attribute_string(stream, ".", attr, value_get_string(value)) ;
    break ;
   default:
    break ;
    }

  return status ;
  }

/*** UNUSED ???
static void set_attribute(Resource *obj, hid_t stream, const char *attr, Value *value)
//====================================================================================
{
  int known = (H5Aexists(stream, attr) > 0) ;

  if (value) {
    dict_set_pointer(obj->metadata, attr, value, 0, NULL) ;  // *************
    if (known) H5Adelete(stream, attr) ;
    set_stream_attribute(stream, attr, value) ;
    }
  else if (known) H5Adelete(stream, attr) ;
  }
****/


static char *get_stream_string_attribute(hid_t stream, const char *attr)
//======================================================================
{
  if (H5LTfind_attribute(stream, attr) == 1) {
    hsize_t attr_dims ;
    H5T_class_t attr_type ;
    size_t attr_size ;
    H5LTget_attribute_info(stream, ".", attr, &attr_dims, &attr_type, &attr_size) ;
    char *value = (char *)calloc(attr_size + 1, 1) ;
    H5LTget_attribute_string(stream, ".", attr, value) ;
    return value ;
    }
  return NULL ;
  }


static void set_clock(hid_t stream, const char *attr, Value *value)
//=================================================================
{
  if ((strcmp(attr, "clock") == 0 || strcmp(attr, "rate") == 0)) {
    if      (value)                     set_stream_attribute(stream, attr, value) ;
    else if (H5Aexists(stream, attr) > 0) H5Adelete(stream, attr) ;
    }
  }

static void del_clock(hid_t stream, const char *attr)
//===================================================
{
  if ((strcmp(attr, "clock") == 0 || strcmp(attr, "rate") == 0)) {
    if (H5Aexists(stream, attr) > 0) H5Adelete(stream, attr) ;
    }
  }


static bsml_graph *read_metadata(hid_t h5, const char *baseuri)
//=============================================================
{
  hsize_t dims ;
  H5T_class_t class ;
  size_t size ;
  if (H5LTfind_dataset(h5, DATASET_META) == 0)
    return NULL ;                         // No 'metadata' dataset

  int error = 1 ;
  char *format = "" ;
  hid_t md = H5Dopen(h5, DATASET_META, H5P_DEFAULT) ;
  if (H5LTfind_attribute(md, "format")) {
    H5LTget_attribute_info(h5, DATASET_META, "format", &dims, &class, &size) ;
    format = calloc(size + 1, 1) ;
    H5LTget_attribute_string(h5, DATASET_META, "format", format) ;
    error = 0 ;
    }
  H5Dclose(md) ;
  if (error) return NULL ;                // No 'format'

  H5LTget_dataset_info(h5, DATASET_META, &dims, &class, &size) ;
  char *rdf = calloc(size + 1, 1) ;
  H5LTread_dataset_string(h5, DATASET_META, rdf) ;
  bsml_graph *g = bsml_graph_create_from_string(rdf, format, baseuri) ;
  free(rdf) ;

  return g ;
  }


static int find_recording(int index, Value *v, void *userdata)
//============================================================
{
  int stopping = 0 ;
  bsml_recording *r = (bsml_recording *)userdata ;
  STREAMRecInfo *h5info = r->info ;
  int kind ;
  bsml_recording *rec = (bsml_recording *)value_get_pointer(v, &kind) ;
  if (kind == BSML_KIND_RECORDING && (r->uri == NULL || strcmp(r->uri, rec->uri) == 0)) {
    char *name = normalise_name(rec->uri) ;
    if (H5Lexists(h5info->file, name, H5P_DEFAULT) == TRUE) {
      hsize_t dims ;
      H5T_class_t class ;
      size_t size ;
      if (H5LTget_attribute_info(h5info->file, name, "uri", &dims, &class, &size) >= 0) {
        char *uri = calloc(size + 1, 1) ;
        H5LTget_attribute_string(h5info->file, name, "uri", uri) ;
        if (strcmp(rec->uri, uri) == 0) {
          h5info->recording = H5Gopen(h5info->file, name, H5P_DEFAULT) ;
          if (r->uri == NULL) r->uri = string_copy(uri) ;
          stopping = 1 ;        // Have first group with uri that matches a Recording
          }
        free(uri) ;
        }
      }
    free(name) ;
    }
  return stopping ;
  }


bsml_recording *bsml_stream_recording_init(const char *fname, char mode, const char *uri, dict *attributes)
//=========================================================================================================
{
  if (attributes == NULL) attributes = dict_create() ;
  // Check for and remove any 'uri' in attributes
  if (dict_get_value(attributes, "uri", NULL)) dict_delete(attributes, "uri") ;
  dict_set_string(attributes, "format", BSML_STREAM) ;

  bsml_recording *r = bsml_file_recording_init(fname, uri, attributes, BSML_RECORDING_STREAM) ;

  hid_t h5f = (mode == 'w') ? H5Fcreate(fname, H5F_ACC_EXCL,  H5P_DEFAULT, H5P_DEFAULT)
            : (mode == '-') ? H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)
            : (mode == 'a') ? H5Fopen(fname, H5F_ACC_RDWR,   H5P_DEFAULT)
            :                 H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT) ;
  const char *version = get_stream_string_attribute(h5f, "version") ;
  if (version == NULL) {
    version = string_copy(BSML_STREAM_VERSION) ;
    if (strchr("w-a", mode)) H5LTset_attribute_string(h5f, ".", "version", version) ;
    }
  STREAMRecInfo *rec = (STREAMRecInfo *)calloc(sizeof(STREAMRecInfo), 1) ;
  rec->file = h5f ;
  rec->version = version ;
  r->info = (void *)rec ;

  int error = 1 ;
  while (1) {
    if (h5f < 0) break ;
    if (strchr("w-", mode) == NULL) {   // Existing file, check it's valid
      r->graph = read_metadata(h5f, r->uri) ;
      if (r->graph == NULL) break ;
      list *recs = bsml_model_recordings(r->graph) ;
      if (uri == NULL) r->uri = NULL ;  // Will now be set from recording
      list_iterate_break(recs, find_recording, (void *)r) ;
      list_free(recs) ;
      if (rec->recording <= 0) break ;
      }
    error = 0 ;
    break ;   // Reset error flag and continue...
    }
  if (error) {
    bsml_stream_recording_close(r) ;
    return NULL ;
    }

/*
  We identify the file as having a group of signals if the recording group:

  * Has a 2-D numeric dataset called './data'.

  * Has a 1-D string dataset called './uris'.

*/

  if (strchr("w-", mode)) {   // New file
    char *name = normalise_name(uri) ;
    rec->recording = H5Gcreate(h5f, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) ;
    free(name) ;
    H5LTset_attribute_string(rec->recording, ".", "uri", uri) ;
    int buf[1] = { -1 } ;
    H5LTset_attribute_int(rec->recording, ".", "channels", buf, 1) ;
    rec->channels = -1 ;
    }
//  else ...     // Do we read these from an existing file??
  rec->sigdata = -1 ;
  rec->uris = NULL ;

  r->type = BSML_RECORDING_STREAM ;
  return r ;
  }


void bsml_stream_recording_close(bsml_recording *r)
//=================================================
{
  if (r->type == BSML_RECORDING_STREAM) {
    STREAMRecInfo *rec = (STREAMRecInfo *)r->info ;
    if (rec->uris) {
      char **u = rec->uris ;
      while (*u) free(*u++) ;
      }
    if (rec->sigdata > 0) H5Dclose(rec->sigdata) ;
    if (rec->recording > 0) H5Gclose(rec->recording) ;
    if (rec->file > 0) H5Fclose(rec->file) ;
    if (rec->version) free((void *)rec->version) ;
    free(rec) ;
    r->type = -1 ;
    }
  bsml_file_recording_close(r) ;
  }


bsml_recording *bsml_stream_recording_open(const char *fname, char mode, const char *uri)
//=======================================================================================
{
  if (mode == 0) mode = 'r' ;

  /*
    # If multiple recordings per file then can we add extra recordings?
    # With no extension of existing recordings??

    # read/parse metadata and add to self._graph (self._model ??)
    self = cls(fname, mode, uri=uri, metadata=

    # and then add all Signals to Recording...
   */

  // rec->uris  char **
  // rec->sigdata
  // "channels" attribute...

  return bsml_stream_recording_init(fname, mode, uri, NULL) ;
  }


bsml_recording *bsml_stream_recording_create(const char *fname, char mode, const char *uri, dict *attributes)
//===========================================================================================================
{
  if (mode == 0) mode = 'w' ;
  return bsml_stream_recording_init(fname, mode, uri, attributes) ;
  }


// create_from_recording sets source to recording.uri? or recording.source??

bsml_recording *bsml_stream_recording_create_from_recording(bsml_recording *r, const char *fname, char mode)
//==========================================================================================================
{
  if (mode == 0) mode = 'w' ;
  return bsml_stream_recording_init(fname, mode, r->uri, bsml_recording_get_metavars(r)) ;
  }


bsml_signal **bsml_stream_recording_create_signals(bsml_recording *r, char **uris, bsml_timeseries **data)
//========================================================================================================
{
  // if data is not None and len(data.shape) > 1 and len(uris) != data.shape[0]:
  //   raise Exception, "Number of Signal uris different from number of data columns"
  bsml_signal **signals = calloc(sizeof(bsml_signal *), array_len(uris)) ;
  bsml_signal **sp = signals ;
  while (*uris)
    *sp++ = bsml_stream_signal_create(*uris++, r, data ? *data++ : NULL, NULL) ;
  return signals ;
  }


bsml_signal **bsml_stream_recording_create_signal_group(bsml_recording *r, char **uris,
                                                        double **data, int datalen, dict *attributes)
//===================================================================================================
{
  int channels = array_len(uris) ;
  int chanattr[1] = { channels } ;
  hsize_t shape[2] = { channels, 0 } ;

  // if data is not None and len(data.shape) > 1 and channels != data.shape[0]:
  //   raise Exception, "Number of Signal uris different from number of data columns"

  STREAMRecInfo *rec = (STREAMRecInfo *)r->info ;
  H5LTset_attribute_int(rec->recording, ".", "channels", chanattr, 1) ;
  rec->channels = channels ;

  hid_t strtype = H5Tcopy(H5T_C_S1) ;
  H5Tset_size(strtype, H5T_VARIABLE) ;
  H5LTmake_dataset(rec->recording, DATASET_URIS, 1, shape, strtype, uris) ;
  H5Tclose(strtype) ;

  hsize_t limit[2] = { channels, H5S_UNLIMITED } ;
  hid_t dataspace = H5Screate_simple(2, shape, limit) ;
  hsize_t chunk[2] = { channels, STREAM_CHUNKSIZE } ;  // CHUNKSIZE/channels
  hid_t plist = H5Pcreate(H5P_DATASET_CREATE) ;
  H5Pset_chunk(plist, 2, chunk) ;

  hid_t dataset = H5Dcreate(rec->recording, DATASET_DATA, H5T_NATIVE_DOUBLE,
                            dataspace, H5P_DEFAULT, plist, H5P_DEFAULT) ;

  H5Sclose(dataspace) ;
  H5Pclose(plist) ;
  rec->sigdata = dataset ;

  int n = 0 ;
  bsml_signal **signals = calloc(sizeof(bsml_signal *), channels) ;
  while (*uris) {
    signals[n] = bsml_stream_signal_init(*uris++, r, dataset, attributes, n) ;
    ++n ;
    }
  if (data) bsml_stream_recording_append_signal_data(r, data, datalen) ;
  return signals ;
  }



void bsml_stream_recording_append_signal_data(bsml_recording *r, double **data, int datalen)
//==========================================================================================
{
  STREAMRecInfo *rec = (STREAMRecInfo *)r->info ;
  if (rec->sigdata >= 0) {
    if (datalen <= 0) return ;

    hid_t dataspace = H5Dget_space(rec->sigdata) ;
    hsize_t dims[H5Sget_simple_extent_ndims(dataspace)] ;
    H5Sget_simple_extent_dims(dataspace, dims, NULL) ;

    hsize_t start[2], count[2] ;
    start[0] = 0 ;
    start[1] = dims[1] ;

    count[0] = dims[0] ;
    count[1] = datalen ;

    dims[1] += datalen ;
    H5Dset_extent(rec->sigdata, dims) ;
    H5Sclose(dataspace) ;

    dataspace = H5Dget_space(rec->sigdata) ;    // Now have larger dataspace
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL, count, NULL) ;
    hid_t memspace = H5Screate_simple(2, count, count) ;
    H5Dwrite(rec->sigdata, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, data) ;

    H5Sclose(dataspace) ;
    H5Sclose(memspace) ;
    }
  // else raise Exception, "Recording has no Signal group"
  }


void bsml_stream_recording_save_metadata(bsml_recording *r, const char *format, dict *prefixes)
//=============================================================================================
{
  if (format == NULL) format = "turtle" ;
  char *rdf = bsml_recording_metadata_as_string(r, format, prefixes) ;
  hid_t h5f = ((STREAMRecInfo *)r->info)->file ;

  if (H5LTfind_dataset(h5f, DATASET_META)) H5Ldelete(h5f, DATASET_META, H5P_DEFAULT) ;
  H5LTmake_dataset_string(h5f, DATASET_META, rdf) ;
  H5LTset_attribute_string(h5f, DATASET_META, "format", format) ;
  free(rdf) ;
  }


/**
 * STREAM Signals
 *
 **/

bsml_signal *bsml_stream_signal_init(const char *uri, bsml_recording *r, hid_t dataset, dict *attributes, int index)
//==================================================================================================================
{
  if (attributes == NULL) attributes = dict_create() ;
  if (dict_get_value(attributes, "uri", NULL)) dict_delete(attributes, "uri") ;
  bsml_signal *s = bsml_file_signal_init(uri, attributes, BSML_SIGNAL_STREAM) ;

  STREAMSigInfo *sig = (STREAMSigInfo *)calloc(sizeof(STREAMRecInfo), 1) ;
  sig->dataset = dataset ;
  sig->index = index ;
  if (index < 0) H5LTset_attribute_string(dataset, ".", "uri", uri) ;
  s->info = (char *)sig ;

  bsml_recording_add_signal(r, s) ;
  return s ;
  }

/**********************
  def __setattr__(self, attr, value):
  #----------------------------------
    _set_clock(self._dset, attr, value)

static void set_clock(dataset, const char *attr, Value *value)

    object.__setattr__(self, attr, value)

  def __delattr__(self, attr):
  #---------------------------
    _del_clock(self._dset, attr)
    object.__delattr__(self, attr)

******************/



void bsml_stream_signal_close(bsml_signal *s)
//===========================================
{
  H5Dclose(((STREAMSigInfo *)s->info)->dataset) ;
  bsml_file_signal_close(s) ;
  }


bsml_signal *bsml_stream_signal_open(const char *uri, bsml_recording *r)
//======================================================================
{
  STREAMRecInfo *rec = (STREAMRecInfo *)r->info ;

  if (rec->sigdata < 0) {
    char *name = make_name(r->uri, uri) ;
    hid_t dataset = H5Dopen(rec->recording, name, H5P_DEFAULT) ;
    free(name) ;
    char *siguri = get_stream_string_attribute(dataset, "uri") ;
    if (strcmp(uri, siguri) == 0) {
      free(siguri) ;
      return bsml_stream_signal_init(uri, r, dataset, NULL, -1) ;  // attributes, -1) ;
                                                           // get from metadata block...
                                                           // via dataset.attrs['uri']
      }
    else {
      free(siguri) ;
      return NULL ;  // error....
      }
    }
  else {
    int index = 0 ;
    char **u = rec->uris ;

    // r->channels ...
    while (*u && strcmp(uri, *u) != 0) ++u, ++index ;

    if (u) return bsml_stream_signal_init(uri, r, rec->sigdata, NULL, index) ; // attributes, index) ;
    else {
      //raise ValueError, 'Signal %s is not in Recording' % str(uri)
      return NULL ;
      }
    }
  }


bsml_signal *bsml_stream_signal_create(const char *uri, bsml_recording *r, bsml_timeseries *data, dict *attributes)
//=================================================================================================================
{
  if (attributes == NULL) attributes = dict_create() ;

  hsize_t shape[1] = { 0 } ;
  hsize_t limit[1] = { H5S_UNLIMITED } ;
  hid_t dataspace = H5Screate_simple(1, shape, limit) ;

  hsize_t chunk[1] = { STREAM_CHUNKSIZE } ;
  hid_t plist = H5Pcreate(H5P_DATASET_CREATE) ;
  H5Pset_chunk(plist, 1, chunk) ;

  char *name = make_name(r->uri, uri) ;
  hid_t dataset = H5Dcreate(((STREAMRecInfo *)r->info)->recording, name, H5T_NATIVE_DOUBLE,
                            dataspace, H5P_DEFAULT, plist, H5P_DEFAULT) ;
  free(name) ;
  H5Sclose(dataspace) ;
  H5Pclose(plist) ;

  bsml_signal *s = bsml_stream_signal_init(uri, r, dataset, attributes, -1) ;
  if (data) bsml_stream_signal_append(s, data) ;
  return s ;
  }


bsml_signal *bsml_stream_signal_create_from_signal(bsml_signal *s, bsml_recording *r)
//===================================================================================
{
  return bsml_stream_signal_create(s->uri, r, NULL, bsml_signal_get_metavars(s)) ;
  }


void bsml_stream_signal_append(bsml_signal *s, bsml_timeseries *data)
//===================================================================
{
 /*
  :param data: TimeSeries of data points
  :type data: :class:`TimeSeries`

 */
  STREAMSigInfo *sig = (STREAMSigInfo *)s->info ;

  if (sig->index < 0) {

    if (data->len <= 0) return ;

    hid_t dataspace = H5Dget_space(sig->dataset) ;
    hsize_t dims[H5Sget_simple_extent_ndims(dataspace)] ;
    H5Sget_simple_extent_dims(dataspace, dims, NULL) ;

    hsize_t start[1], count[1] ;
    start[0] = dims[0] ;
    count[0] = data->len ;

    dims[0] += data->len ;
    H5Dset_extent(sig->dataset, dims) ;
    H5Sclose(dataspace) ;
    
    dataspace = H5Dget_space(sig->dataset) ;    // Now have larger dataspace
    H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL, count, NULL) ;
    hid_t memspace = H5Screate_simple(1, count, count) ;
    H5Dwrite(sig->dataset, H5T_NATIVE_DOUBLE, memspace, dataspace, H5P_DEFAULT, data->data) ;

    H5Sclose(dataspace) ;
    H5Sclose(memspace) ;
    }
  // else: raise Exception, "Cannot append to individual signals in a group"
  }


/*


  def read(self, interval=None, segment=None, duration=None, points=0):
  #--------------------------------------------------------------------
    # Compute indices into dataset
    # return self._dset[start:stop] ## But yield a sequence of TimeSeries??
    if self._index is None: return self._dest[start:stop]
    else:                   return self._dset[self._index, start:stop]

  def __getitem__(self, key):
  #--------------------------
    if self._index is None: return self._dest[key]
    else:                   return self._dset[self._index, key]

*/



#ifdef STREAMTEST

int main(void)
//============
{

  bsml_rdf_initialise() ;

//  H5Eset_auto(H5E_DEFAULT, error_fn, NULL) ;  // ** Our error reporting...

  char *hdf = "/Users/dave/biosignalml/libbsml/src/api/edf.h5" ;
  bsml_recording *stream = bsml_stream_recording_open(hdf, 'r', NULL) ;


  dict_print(stream->attributes) ;
  // Get/add signals? Part of recording open??

  //list *sigs = bsml_recording_signals(stream) ;  // v's list of STREAMSignals

  // Or dict, indexed by sig.uri ??


  bsml_stream_recording_close(stream) ;

  // H5close() ;   // ???????????

  }

#endif
