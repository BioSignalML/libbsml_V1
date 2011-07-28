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

  A single Recording per HDF5 file, either with separate datasets for
  each Signal of the Recording::

    Path               Object   Type     Attributes
    ----               ------   ----     ----------
    /                  Root
    /metadata          Dataset  STRING   format
    /recording_uri     Group             uri
     ./signal0_uri     Dataset  Numeric  uri, rate/clock
     ./signal1_uri     Dataset  Numeric  uri, rate/clock
         .
     ./clock0_uri      Dataset  Numeric  uri
         .

  and second case with simulation data inside a ./signals group::

    Path               Object   Type     Attributes
    ----               ------   ----     ----------
    /                  Root
    /metadata          Dataset  STRING   format
    /simulation_uri    Group             uri
     ./data            Dataset  Numeric  rate/clock
     ./uris            Dataset  STRING

**/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include <hdf5.h>
#include <hdf5_hl.h>

#include "bsml_hdf5.h"

// Helper functions:

static char *normalise_name(const char *name)
/*=========================================*/
{
  char *nm = string_copy(name) ;
  char *s = nm ;
  while (1) {
    s = strchr(s, '/') ;
    if (s) *s = '_' ;
    else break ;
    }
  return nm ;
  }

static char *make_name(const char *prefix, const char *uri)
/*=======================================================*/
{
  if (strstr(uri, prefix) == uri) {
    uri += strlen(prefix) ;
    if (*uri == '/') uri += 1 ;
    }
  return normalise_name(uri) ;
  }


static int array_len(char **array)
/*==============================*/
{
  int n = 0 ;
  while (*array++) ++n ;
  return n ;
  }


static herr_t set_hdf5_attribute(hid_t hdf5, const char *attr, Value *value)
/*========================================================================*/
{
  herr_t status = 0 ;

  switch (value->type) {
    case TYPE_SHORT: {
      short val = (short)value->integer ;
      status = H5LTset_attribute_short(hdf5, ".", attr, &val, 1) ;
      break ;
      }
    case TYPE_INTEGER: {
      int val = (int)value->integer ;
      status = H5LTset_attribute_int  (hdf5, ".", attr, &val, 1) ;
      break ;
      }
    case TYPE_LONG: {
      long val = (long)value->integer ;
      status = H5LTset_attribute_long (hdf5, ".", attr, &val, 1) ;
      break ;
      }
    case TYPE_FLOAT: {
      float val = (float)value->real ;
      status = H5LTset_attribute_float (hdf5, ".", attr, &val, 1) ;
      break ;
      }
    case TYPE_DOUBLE: {
      double val = (double)value->real ;
      status = H5LTset_attribute_double(hdf5, ".", attr, &val, 1) ;
      break ;
      }
   case TYPE_STRING:
    status = H5LTset_attribute_string(hdf5, ".", attr, value->string) ;
    break ;

   default:
    break ;
    }

  return status ;
  }

static void set_attribute(Resource *obj, hid_t hdf5, const char *attr, Value *value)
/*================================================================================*/
{
  int known = (H5Aexists(hdf5, attr) > 0) ;

  if (value) {
    bsml_resource_set_value(obj->metadata, attr, value) ;
    if (known) H5Adelete(hdf5, attr) ;
    set_hdf5_attribute(hdf5, attr, value) ;
    }
  else if (known) H5Adelete(hdf5, attr) ;
  }

static char *get_hdf5_string_attribute(hid_t hdf5, const char *attr)
/*================================================================*/
{
  hsize_t attr_dims ;
  H5T_class_t attr_type ;
  size_t attr_size ;

  H5LTget_attribute_info(hdf5, ".", attr, &attr_dims, &attr_type, &attr_size) ;
  char *value = (char *)calloc(attr_size + 1, 1) ;
  H5LTget_attribute_string(hdf5, ".", attr, value) ;
  return value ;
  }


static void set_clock(hid_t hdf5, const char *attr, Value *value)
/*=============================================================*/
{
  if ((strcmp(attr, "clock") == 0 || strcmp(attr, "rate") == 0)) {
    if      (value)                     set_hdf5_attribute(hdf5, attr, value) ;
    else if (H5Aexists(hdf5, attr) > 0) H5Adelete(hdf5, attr) ;
    }
  }

static void del_clock(hid_t hdf5, const char *attr)
/*===============================================*/
{
  if ((strcmp(attr, "clock") == 0 || strcmp(attr, "rate") == 0)) {
    if (H5Aexists(hdf5, attr) > 0) H5Adelete(hdf5, attr) ;
    }
  }


Recording *HDF5Recording_init(const char *fname, char mode, const char *uri, Dictionary *attributes)
/*==============================================================================================*/
{
  if (attributes == NULL) attributes = dict_create() ;
  if (dict_get(attributes, "uri")) dict_delete(attributes, "uri") ; // In case attributes contains a uri
  dict_set_string(attributes, "format", BSML.HDF5) ;
  Recording *r = FILERecording_init(fname, uri, attributes, RECORDING_HDF5) ;

  HDF5RecInfo *rec = (HDF5RecInfo *)calloc(sizeof(HDF5RecInfo), 1) ;
  rec->file = (mode == 'w') ? H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)
            : (mode == '-') ? H5Fcreate(fname, H5F_ACC_EXCL,  H5P_DEFAULT, H5P_DEFAULT)
            : (mode == 'a') ? H5Fopen(fname, H5F_ACC_RDWR,   H5P_DEFAULT)
            :                 H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT) ;
  char *name = normalise_name(uri) ;
  rec->recording = H5Gopen(rec->file, name, H5P_DEFAULT) ;
  if (rec->recording < 0)
    rec->recording = H5Gcreate(rec->file, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) ;
  free(name) ;
  H5LTset_attribute_string(rec->recording, ".", "uri", uri) ;
  int buf[1] = { -1 } ;
  H5LTset_attribute_int(rec->recording, ".", "channels", buf, 1) ;
  rec->channels = -1 ;
  rec->sigdata = -1 ;
  rec->uris = NULL ;

  r->info = (void *)rec ;
  r->type = RECORDING_HDF5 ;
  return r ;
  }


void HDF5Recording_close(Recording *r)
/*==================================*/
{
  if (r->type == RECORDING_HDF5) {
    HDF5RecInfo *rec = (HDF5RecInfo *)r->info ;
    if (rec->uris) {
      char **u = rec->uris ;
      while (*u) free(*u++) ;
      }
    if (rec->sigdata >= 0) H5Dclose(rec->sigdata) ;
    if (rec->recording >= 0) H5Gclose(rec->recording) ;
    if (rec->file >= 0) H5Fclose(rec->file) ;
    free(rec) ;
    r->type = -1 ;
    }
  FILERecording_close(r) ;
  }



Recording *HDF5Recording_open(const char *fname, char mode, const char *uri)
/*========================================================================*/
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

  return HDF5Recording_init(fname, mode, uri, NULL) ;
  }


Recording *HDF5Recording_create(const char *fname, char mode, const char *uri, Dictionary *attributes)
/*================================================================================================*/
{
  if (mode == 0) mode = '-' ;
  return HDF5Recording_init(fname, mode, uri, attributes) ;
  }


// create_from_recording sets source to recording.uri? or recording.source??

Recording *HDF5Recording_create_from_recording(Recording *r, const char *fname, char mode)
/*======================================================================================*/
{
  if (mode == 0) mode = '-' ;
  return HDF5Recording_init(fname, mode, r->uri, Recording_get_metavars(r)) ;
  }


Signal **HDF5Recording_create_signals(Recording *r, char **uris, TimeSeries **data)
/*===============================================================================*/
{
  // if data is not None and len(data.shape) > 1 and len(uris) != data.shape[0]:
  //   raise Exception, "Number of Signal uris different from number of data columns"
  Signal **signals = calloc(sizeof(Signal *), array_len(uris)) ;
  Signal **sp = signals ;
  while (*uris)
    *sp++ = HDF5Signal_create(*uris++, r, data ? *data++ : NULL, NULL) ;
  return signals ;
  }


Signal **HDF5Recording_create_signal_group(Recording *r, char **uris, double **data, int datalen,
                                                                           Dictionary *attributes)
/*==============================================================================================*/
{
  int channels = array_len(uris) ;
  int chanattr[1] = { channels } ;
  hsize_t shape[2] = { channels, 0 } ;

  // if data is not None and len(data.shape) > 1 and channels != data.shape[0]:
  //   raise Exception, "Number of Signal uris different from number of data columns"

  HDF5RecInfo *rec = (HDF5RecInfo *)r->info ;
  H5LTset_attribute_int(rec->recording, ".", "channels", chanattr, 1) ;
  rec->channels = channels ;

  hid_t strtype = H5Tcopy(H5T_C_S1) ;
  H5Tset_size(strtype, H5T_VARIABLE) ;
  H5LTmake_dataset(rec->recording, "uris", 1, shape, strtype, uris) ;
  H5Tclose(strtype) ;

  hsize_t limit[2] = { channels, H5S_UNLIMITED } ;
  hid_t dataspace = H5Screate_simple(2, shape, limit) ;
  hsize_t chunk[2] = { channels, HDF5_CHUNKSIZE } ;  // CHUNKSIZE/channels
  hid_t plist = H5Pcreate(H5P_DATASET_CREATE) ;
  H5Pset_chunk(plist, 2, chunk) ;

  hid_t dataset = H5Dcreate(rec->recording, "data", H5T_NATIVE_DOUBLE,
                            dataspace, H5P_DEFAULT, plist, H5P_DEFAULT) ;

  H5Sclose(dataspace) ;
  H5Pclose(plist) ;
  rec->sigdata = dataset ;

  int n = 0 ;
  Signal **signals = calloc(sizeof(Signal *), channels) ;
  while (*uris) {
    signals[n] = HDF5Signal_init(*uris++, r, dataset, attributes, n) ;
    ++n ;
    }
  if (data) HDF5Recording_append_signal_data(r, data, datalen) ;
  return signals ;
  }



void HDF5Recording_append_signal_data(Recording *r, double **data, int datalen)
/*===========================================================================*/
{
  HDF5RecInfo *rec = (HDF5RecInfo *)r->info ;
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


void HDF5Recording_save_metadata(Recording *r, const char *format, Dictionary *prefixes)
/*====================================================================================*/
{
  if (format == NULL) format = "turtle" ;
  char *rdf = Recording_metadata_as_string(r, format, prefixes) ;
  hid_t md = H5LTmake_dataset_string(((HDF5RecInfo *)r->info)->file, "metadata", rdf) ;
  H5LTset_attribute_string(md, ".", "format", format) ;
  H5Dclose(md) ;
  free(rdf) ;
  }


/**
 * HDF5 Signals
 *
 **/

Signal *HDF5Signal_init(const char *uri, Recording *r, hid_t dataset, Dictionary *attributes, int index)
/*==================================================================================================*/
{
  if (attributes == NULL) attributes = dict_create() ;
  if (dict_get(attributes, "uri")) dict_delete(attributes, "uri") ; // In case attributes contains a uri
  Signal *s = FILESignal_init(uri, attributes, SIGNAL_HDF5) ;

  HDF5SigInfo *sig = (HDF5SigInfo *)calloc(sizeof(HDF5RecInfo), 1) ;
  sig->dataset = dataset ;
  sig->index = index ;
  if (index < 0) H5LTset_attribute_string(dataset, ".", "uri", uri) ;
  s->info = (char *)sig ;

  HDF5Recording_add_signal(r, s) ;
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



void HDF5Signal_close(Signal *s)
/*============================*/
{
  H5Dclose(((HDF5SigInfo *)s->info)->dataset) ;
  FILESignal_close(s) ;
  }


Signal *HDF5Signal_open(const char *uri, Recording *r)
/*==================================================*/
{
  HDF5RecInfo *rec = (HDF5RecInfo *)r->info ;

  if (rec->sigdata < 0) {
    char *name = make_name(r->uri, uri) ;
    hid_t dataset = H5Dopen(rec->recording, name, H5P_DEFAULT) ;
    free(name) ;
    char *siguri = get_hdf5_string_attribute(dataset, "uri") ;
    if (strcmp(uri, siguri) == 0) {
      free(siguri) ;
      return HDF5Signal_init(uri, r, dataset, NULL, -1) ;  // attributes, -1) ;
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

    if (u) return HDF5Signal_init(uri, r, rec->sigdata, NULL, index) ; // attributes, index) ;
    else {
      //raise ValueError, 'Signal %s is not in Recording' % str(uri)
      return NULL ;
      }
    }
  }


Signal *HDF5Signal_create(const char *uri, Recording *r, TimeSeries *data, Dictionary *attributes)
/*==============================================================================================*/
{
  if (attributes == NULL) attributes = dict_create() ;

  hsize_t shape[1] = { 0 } ;
  hsize_t limit[1] = { H5S_UNLIMITED } ;
  hid_t dataspace = H5Screate_simple(1, shape, limit) ;

  hsize_t chunk[1] = { HDF5_CHUNKSIZE } ;
  hid_t plist = H5Pcreate(H5P_DATASET_CREATE) ;
  H5Pset_chunk(plist, 1, chunk) ;

  char *name = make_name(r->uri, uri) ;
  hid_t dataset = H5Dcreate(((HDF5RecInfo *)r->info)->recording, name, H5T_NATIVE_DOUBLE,
                            dataspace, H5P_DEFAULT, plist, H5P_DEFAULT) ;
  free(name) ;
  H5Sclose(dataspace) ;
  H5Pclose(plist) ;

  Signal *s = HDF5Signal_init(uri, r, dataset, attributes, -1) ;
  if (data) HDF5Signal_append(s, data) ;
  return s ;
  }


Signal *HDF5Signal_create_from_signal(Signal *s, Recording *r)
/*==========================================================*/
{
  return HDF5Signal_create(s->uri, r, NULL, Signal_get_metavars(s)) ;
  }


void HDF5Signal_append(Signal *s, TimeSeries *data)
/*===============================================*/
{
 /*
  :param data: TimeSeries of data points
  :type data: :class:`TimeSeries`

 */
  HDF5SigInfo *sig = (HDF5SigInfo *)s->info ;

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
