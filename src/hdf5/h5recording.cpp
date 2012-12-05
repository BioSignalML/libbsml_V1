/******************************************************************************
 *                                                                            *
 *  BioSignalML Management in C++                                             *
 *                                                                            *
 *  Copyright (c) 2010-2012  David Brooks                                     *
 *                                                                            *
 *  Licensed under the Apache License, Version 2.0 (the "License");           *
 *  you may not use this file except in compliance with the License.          *
 *  You may obtain a copy of the License at                                   *
 *                                                                            *
 *      http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                            *
 *  Unless required by applicable law or agreed to in writing, software       *
 *  distributed under the License is distributed on an "AS IS" BASIS,         *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *  See the License for the specific language governing permissions and       *
 *  limitations under the License.                                            *
 *                                                                            *
 ******************************************************************************/

#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>
#include <list>

#include <H5Cpp.h>

#include "bsml_h5.h"

using namespace BSML ;

//#define H5_DEBUG 1

extern "C" {
  static herr_t saveSignal(hid_t, const char *, void *) ;
  static herr_t saveClock(hid_t, const char *, void *) ;
  } ;

typedef struct {
  H5::H5File h5 ;
  void *listp ;
  } SaveInfo ;


H5Recording BSML::H5create(const std::string &uri, const std::string &fname, bool replace=false)
/*============================================================================================*/
{
//Create a new HDF5 Recording file.
//
//:param uri: The URI of the Recording contained in the file.
//:param fname: The name of the file to create.
//:type fname: str
//:param replace: If True replace any existing file (default = False).
//:param replace: bool
  try {
#if !H5_DEBUG
    H5::Exception::dontPrint() ;
#endif
    H5::H5File h5 = H5::H5File(fname, replace ? H5F_ACC_TRUNC : H5F_ACC_EXCL) ;

    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    H5::DataSpace scalar(H5S_SCALAR) ;
    H5::Attribute attr ;

    H5::Group root = h5.openGroup("/") ;
    H5::Group uris = h5.createGroup("/uris") ;
    H5::Group rec = h5.createGroup("/recording") ;
    h5.createGroup("/recording/signal") ;
    
    attr = root.createAttribute("version", varstr, scalar) ;
    attr.write(varstr, BSML_H5_VERSION) ;
    attr.close() ;

    attr = rec.createAttribute("uri", varstr, scalar) ;
    attr.write(varstr, uri) ;
    attr.close() ;

    hobj_ref_t ref ;
    attr = uris.createAttribute(uri, H5::PredType::STD_REF_OBJ, scalar) ;
    h5.reference(&ref, "/recording") ;
    attr.write(H5::PredType::STD_REF_OBJ, &ref) ;
    attr.close() ;

    h5.flush(H5F_SCOPE_GLOBAL) ;
    return H5Recording(uri, h5) ;
    }
  catch (H5::FileIException e) {
    throw H5Exception("Cannot create '" + fname + "': " + e.getDetailMsg()) ;
    }
  }


H5Recording BSML::H5open(const std::string &fname, bool readonly=false)
/*===================================================================*/
{
//Open an existing HDF5 Recording file.
//
//:param fname: The name of the file to open.
//:type fname: str
//:param readonly: If True don't allow updates (default = False).
//:param readonly: bool
  H5::H5File h5 ;
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  try {
    std::string fn = (!fname.compare(0, 7, "file://")) ? fname.substr(7) : fname ;
    h5 = H5::H5File(fn, readonly ? H5F_ACC_RDONLY : H5F_ACC_RDWR) ;
    }
  catch (H5::FileIException e) {
    throw H5Exception("Cannot open '" + fname + "': " + e.getDetailMsg()) ;
    }

  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  H5::Attribute attr ;
  try {
    std::string version ;
    H5::Group root = h5.openGroup("/") ;
    attr = root.openAttribute("version") ;
    attr.read(varstr, version) ;

    if (version.compare(0, 5, BSML_H5_VERSION, 0, 5))
      throw H5Exception("Invalid BiosignalML x HDF5 file: '" + fname + "'") ;

    int dot = version.find('.', 5) ;
    int h5major, major, minor ;
    if (!(dot != std::string::npos
       && (std::stringstream(version.substr(5, dot)) >> major)
       && (std::stringstream(version.substr(dot + 1)) >> minor)
       && (std::stringstream(BSML_H5_MAJOR) >> h5major)
       && major <= h5major))
      throw H5Exception("File '" + fname + "' not compatible with version " + BSML_H5_VERSION) ;

    H5::Group rec, uris ;
    char *uri = NULL ;

    uris = h5.openGroup("/uris") ;
    rec = h5.openGroup("/recording") ;
    h5.openGroup("/recording/signal") ;

    attr = rec.openAttribute("uri") ;
    attr.read(varstr, &uri) ;
    attr.close() ;

    attr = uris.openAttribute(uri) ;
    hobj_ref_t uriref, recref ;
    attr.read(H5::PredType::STD_REF_OBJ, &uriref) ;
    h5.reference(&recref, "/recording") ;
    attr.close() ;
    if (uriref != recref)
      throw H5Exception("Internal error in BiosignalML HDF5 file: '" + fname + "'") ;
  
    return H5Recording(std::string(uri), h5) ;
    }
  catch (H5::Exception e) {
    throw H5Exception("Invalid BiosignalML HDF5 file: '" + fname + "': " + e.getDetailMsg()) ;
    }
  }


H5Recording::H5Recording(const std::string &uri, H5::H5File h5)
/*===========================================================*/
: uri(uri), h5(h5) { }


H5Recording::~H5Recording(void)
/*===========================*/
{
  close() ;
  }

void H5Recording::close(void)
/*=========================*/
{
  h5.close() ;
  }



H5Clock H5Recording::checkTiming(double rate, double period, const std::string &clock, size_t npoints)
/*==================================================================================================*/
{
  H5Clock clocktimes = H5Clock() ;
  if (rate != 0.0 || period != 0.0) {
    if (clock != "" || rate != 0.0 && period != 0.0)
      throw H5Exception("Only one of 'rate', 'period', or 'clock' can be specified") ;
    }
  else if (clock != "") {
    if (rate != 0.0 || period != 0.0)
      throw H5Exception("Only one of 'rate', 'period', or 'clock' can be specified") ;
    clocktimes = getClock(clock) ;
    if (clocktimes.length() < npoints)
      throw H5Exception("Clock either doesn't exist or have sufficient times") ;
    }
  else {
    throw H5Exception("No timing information given") ;
    }
  return clocktimes ;
  }


H5DataRef H5Recording::createDataset(const std::string &group, int rank, hsize_t *shape,
/*===================================================================================*/
 hsize_t *maxshape, void *data, H5DataTypes datatypes, H5Compression compression)
{
  H5::DataSpace dspace(rank, shape, maxshape) ;
  H5::DataType dtype = datatypes.dtype ;
  if (dtype.getId() == 0) dtype = BSML_H5_DEFAULT_DATATYPE ;

  H5::Group grp ;
  try {
    grp = h5.openGroup("/recording/" + group) ;
    }
  catch (H5::FileIException e) {
    grp = h5.createGroup("/recording/" + group) ;
    }
  std::string dsetname = "/recording/" + group + "/" + to_string(grp.getNumObjs()) ;
  try {
    H5::DSetCreatPropList props ;
    hsize_t chunks[rank] ;
    chunks[0] = 4096 ;
    int chunkbytes = chunks[0]*dtype.getSize() ;
    for (int n = 1 ;  n < rank ;  ++n) {
      chunkbytes *= shape[n] ;
      chunks[n] = shape[n] ;
      }
    while (chunkbytes < BSML_H5_CHUNK_BYTES) {
      chunks[0] *= 2 ;
      chunkbytes *= 2 ;
      }
    props.setChunk(rank, chunks) ;
    if      (compression == BSML_H5_COMPRESS_GZIP) props.setDeflate(4) ;
    else if (compression == BSML_H5_COMPRESS_SZIP) props.setSzip(H5_SZIP_NN_OPTION_MASK, 8) ;
    H5::DataSet dset = h5.createDataSet(dsetname, dtype, dspace, props) ;
    if (data != NULL) dset.write(data, datatypes.mtype) ;
    hobj_ref_t reference ;
    h5.reference(&reference, dsetname) ;
    return H5DataRef(dset, reference) ;
    }
  catch (H5::FileIException e) {
    e.printError() ;
    throw H5Exception("Cannot create '" + group + "' dataset: " + e.getDetailMsg()) ;
    }
  }


void H5Recording::setSignalAttributes(H5::DataSet dset, double gain, double offset,
/*===============================================================================*/
 double rate, double period, const std::string &timeunits, H5Clock clocktimes)
{
  H5::Attribute attr ;
  H5::DataSpace scalar(H5S_SCALAR) ;

  if (gain != 1.0) {
    attr = dset.createAttribute("gain", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &gain) ;
    attr.close() ;
    }
  if (offset != 0.0) {
    attr = dset.createAttribute("offset", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &offset) ;
    attr.close() ;
    }

  if (rate != 0.0) {
    attr = dset.createAttribute("rate", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &rate) ;
    attr.close() ;
    }
  else if (period != 0.0) {
    attr = dset.createAttribute("period", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &period) ;
    attr.close() ;
    }
  else if (clocktimes.length() > 0) {
    attr = dset.createAttribute("clock", H5::PredType::STD_REF_OBJ, scalar) ;
    hobj_ref_t ref = clocktimes.getRef() ;
    attr.write(H5::PredType::STD_REF_OBJ, &ref) ;
    attr.close() ;
    }
  if (timeunits != "") {
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    attr = dset.createAttribute("timeunits", varstr, scalar) ;
    attr.write(varstr, timeunits) ;
    attr.close() ;
    }

  }


H5Signal H5Recording::createSignal(const std::string &uri, const std::string &units,
/*===============================================================================*/
 void *data=NULL, size_t datasize=0, H5DataTypes datatypes=H5DataTypes(),
 std::vector<hsize_t> datashape=std::vector<hsize_t>(),
 double gain=1.0, double offset=0.0, double rate=0.0, double period=0.0,
 const std::string &timeunits="", const std::string &clock="",
 H5Compression compression=BSML_H5_DEFAULT_COMPRESSION)
{
#if !H5_DEBUG
//  H5::Exception::dontPrint() ;
#endif
  H5::Attribute attr ;
  H5::Group urigroup = h5.openGroup("/uris") ;
  try {
    attr = urigroup.openAttribute(uri) ;
    attr.close() ;
    throw H5Exception("A signal already has URI '" + uri + "'") ;
    }
  catch (H5::AttributeIException e) { }
  
  size_t npoints = 0 ;
  int rank = datashape.size() + 1 ;
  hsize_t maxshape[rank], shape[rank] ;
  maxshape[0] = H5S_UNLIMITED ;
  int elsize = 1 ;
  if (rank > 1) {           // simple dataset, shape of data point given
    for (int n = 0 ;  n < (rank - 1) ;  ++n) {
      int count = datashape.at(n) ;
      maxshape[n + 1] = shape[n + 1] = count ;
      elsize *= count ;
      }
    npoints = (data != NULL) ? datasize/elsize : 0 ;
    }
  else if (data != NULL) {  // simple dataset, data determines shape
    npoints = datasize ;
    }
  shape[0] = npoints ;

  H5Clock clocktimes = checkTiming(rate, period, clock, npoints) ;
  H5DataRef sigdata = createDataset("signal", rank, shape, maxshape, data, datatypes, compression) ;
  H5::DataSet dset = sigdata.first ;

  H5::DataSpace scalar(H5S_SCALAR) ;
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  hobj_ref_t reference = sigdata.second ;
  attr = urigroup.createAttribute(uri, H5::PredType::STD_REF_OBJ, scalar) ;
  attr.write(H5::PredType::STD_REF_OBJ, &reference) ;
  attr.close() ;
  attr = dset.createAttribute("uri", varstr, scalar) ;
  attr.write(varstr, uri) ;
  attr.close() ;
  attr = dset.createAttribute("units", varstr, scalar) ;
  attr.write(varstr, units) ;
  attr.close() ;

  setSignalAttributes(dset, gain, offset, period, rate, timeunits, clocktimes) ;
  h5.flush(H5F_SCOPE_GLOBAL) ;
  return H5Signal(uri, sigdata, 0) ;
  }


std::list<H5Signal> H5Recording::createSignal(StringList uris, StringList units,
/*===========================================================================*/
 void *data=NULL, size_t datasize=0, H5DataTypes datatypes=H5DataTypes(),
 double gain=1.0, double offset=0.0, double rate=0.0, double period=0.0,
 const std::string &timeunits="", const std::string &clock="",
 H5Compression compression=BSML_H5_DEFAULT_COMPRESSION)
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  std::list<H5Signal> signals ;

  if (uris.size() == units.size())
    throw H5Exception("'uri' and 'units' have different sizes") ;
  int nsignals = uris.size() ;
  if (nsignals == 1) {
    signals.push_back(createSignal(uris.front(), units.front(),
                                   data, datasize, datatypes, std::vector<hsize_t>(),
                                   gain, offset, rate, period, timeunits, clock, compression)) ;
    return signals ;
    }
  H5::Attribute attr ;
  H5::Group urigroup = h5.openGroup("/uris") ;
  for (StringList::iterator s = uris.begin() ;  s != uris.end() ;  s++) {
    try {
      attr = urigroup.openAttribute(*s) ;
      attr.close() ;
      throw H5Exception("A signal already has URI '" + *s + "'") ;
      }
    catch (H5::AttributeIException e) { }
    }
  size_t npoints = (data != NULL) ? datasize/nsignals : 0 ;
  hsize_t maxshape[2] = { H5S_UNLIMITED, nsignals } ;
  hsize_t shape[2]    = { npoints,       nsignals } ;

  H5Clock clocktimes = checkTiming(rate, period, clock, npoints) ;
  H5DataRef sigdata = createDataset("signal", 2, shape, maxshape, data, datatypes, compression) ;
  H5::DataSet dset = sigdata.first ;

  H5::DataSpace scalar(H5S_SCALAR) ;
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  hobj_ref_t reference = sigdata.second ;
  StringList::iterator s ;
  std::string values[nsignals] ;
  hsize_t dims[1] ;
  dims[0] = nsignals ;
  H5::DataSpace attrspace(1, dims, dims) ;
  int n ;
  for (n = 0, s = uris.begin() ;  s != uris.end() ;  n++, s++) {
    values[n] = *s ;
    attr = urigroup.createAttribute(*s, H5::PredType::STD_REF_OBJ, scalar) ;
    attr.write(H5::PredType::STD_REF_OBJ, &reference) ;
    attr.close() ;
    signals.push_back(H5Signal(*s, sigdata, n)) ;
    }
  attr = dset.createAttribute("uri", varstr, attrspace) ;
  attr.write(varstr, values) ;
  attr.close() ;
  attr = dset.createAttribute("units", varstr, attrspace) ;
  for (n = 0, s = units.begin() ;  s != units.end() ;  n++, s++)
    values[n] = *s ;
  attr.write(varstr, values) ;
  attr.close() ;

  setSignalAttributes(dset, gain, offset, period, rate, timeunits, clocktimes) ;
  h5.flush(H5F_SCOPE_GLOBAL) ;

  return signals ;
  }


H5Clock H5Recording::createClock(const std::string &uri, const std::string &units="",
/*=================================================================================*/
 void *times=NULL, size_t datasize=0, H5DataTypes datatypes=H5DataTypes(),
 double rate=0.0, double period=0.0, H5Compression compression=BSML_H5_DEFAULT_COMPRESSION)
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  H5::Attribute attr ;
  H5::Group urigroup = h5.openGroup("/uris") ;
  try {
    attr = urigroup.openAttribute(uri) ;
    attr.close() ;
    throw H5Exception("A clock already has URI '" + uri + "'") ;
    }
  catch (H5::AttributeIException e) { }

  if (rate != 0.0 && period != 0.0)
    throw H5Exception("Only one of 'rate' or 'period' can be specified") ;
  
  size_t npoints = 0 ;
  hsize_t maxshape[] = { H5S_UNLIMITED } ;
  hsize_t shape[]    = { datasize } ;
  H5DataRef clkdata = createDataset("clock", 1, shape, maxshape, times, datatypes, compression) ;
  H5::DataSet dset = clkdata.first ;

  H5::DataSpace scalar(H5S_SCALAR) ;
  hobj_ref_t reference = clkdata.second ;
  attr = urigroup.createAttribute(uri, H5::PredType::STD_REF_OBJ, scalar) ;
  attr.write(H5::PredType::STD_REF_OBJ, &reference) ;
  attr.close() ;
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  attr = dset.createAttribute("uri", varstr, scalar) ;
  attr.write(varstr, uri) ;
  attr.close() ;
  if (units != "") {
    attr = dset.createAttribute("units", varstr, scalar) ;
    attr.write(varstr, units) ;
    attr.close() ;
    }
  if (rate != 0.0) {
    attr = dset.createAttribute("rate", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &rate) ;
    attr.close() ;
    }
  else if (period != 0.0) {
    attr = dset.createAttribute("period", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &period) ;
    attr.close() ;
    }

  h5.flush(H5F_SCOPE_GLOBAL) ;
  return H5Clock(uri, clkdata) ;
  }


H5DataRef H5Recording::getDataRef(const std::string &uri, const std::string &prefix)
/*================================================================================*/
{
  try {
    H5::Group uris = h5.openGroup("/uris") ;
    H5::Attribute attr = uris.openAttribute(uri) ;
    hobj_ref_t ref ;
    attr.read(H5::PredType::STD_REF_OBJ, &ref) ;
    attr.close() ;
    H5::DataSet dset = H5::DataSet(h5, &ref) ;
    hid_t id = dset.getId() ;
    int len = H5Rget_name(id, H5R_OBJECT, &ref, NULL, 0) ;
    char *buf = (char *)std::malloc(len + 1) ;
    H5Rget_name(id, H5R_OBJECT, &ref, buf, len + 1) ;
    bool matched = (prefix.compare(0, std::string::npos, buf, prefix.size()) == 0) ;
    std::free(buf) ;
    if (matched) return H5DataRef(dset, ref) ;
    }
  catch (H5::AttributeIException e) { }
  return H5DataRef() ;
  }


H5Signal H5Recording::getSignal(const std::string &uri)
/*===================================================*/
{
//Find a signal from its URI.
//
//:param uri: The URI of the signal to get.
//:return: A :class:`H5Signal` containing the signal, or None if
//         the URI is unknown or the dataset is not that for a signal.
  H5DataRef dataref = getDataRef(uri, "/recording/signal/") ;
  if (dataref.first.getId() != 0) {
    H5::DataSet dset = dataref.first ;
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    H5::Attribute attr = dset.openAttribute("uri") ;
    int nsignals = attr.getSpace().getSimpleExtentNpoints() ;
    if (nsignals == 1) {
      std::string dseturi ;
      attr.read(varstr, dseturi) ;
      if (uri == dseturi) return H5Signal(uri, dataref, -1) ;
      }
    else if (nsignals > 1) {
      std::string uris[nsignals] ;
      attr.read(varstr, uris) ;
      int n = 0 ;
      while (n < nsignals) {
        if (uri == uris[n]) return H5Signal(uri, dataref, n) ;
        ++n ;
        }
      }
    }
  throw H5Exception("Cannot find signal:" + uri) ;
  }


H5Clock H5Recording::getClock(const std::string &uri)
/*=================================================*/
{
//Find a clock dataset from its URI.
//
//:param uri: The URI of the clock dataset to get.
//:return: A :class:`H5Clock` or None if the URI is unknown or
//         the dataset is not that for a clock.
  H5DataRef dataref = getDataRef(uri, "/recording/clock/") ;
  if (dataref.first.getId() != 0) return H5Clock(uri, dataref) ;
  throw H5Exception("Cannot find clock:" + uri) ;
  }


static herr_t saveSignal(hid_t id, const char *name, void *op_data)
/*===============================================================*/
{
  SaveInfo *info = (SaveInfo *)op_data ;
  std::list<H5Signal> &sig = reinterpret_cast<std::list<H5Signal> &>(info->listp) ;
  try {
    H5::DataSet dset = H5::DataSet(info->h5.openDataSet(name)) ;
    hobj_ref_t ref ;
    info->h5.reference(&ref, name) ;
    H5DataRef dataref = H5DataRef(dset, ref) ;
    H5::Attribute attr = dset.openAttribute("uri") ;
    int nsignals = attr.getSpace().getSimpleExtentNpoints() ;
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    if (nsignals == 1) {
      std::string uri ;
      attr.read(varstr, uri) ;
      sig.push_back(H5Signal(uri, dataref, -1)) ;
      }
    else if (nsignals > 1) {
      std::string uris[nsignals] ;
      attr.read(varstr, uris) ;
      int n = 0 ;
      while (n < nsignals) {
        sig.push_back(H5Signal(uris[n], dataref, n)) ;
        ++n ;
        }
      }
    }
  catch (H5::FileIException e) { }
  return 0 ;
  }


std::list<H5Signal> H5Recording::signals(void)
/*==========================================*/
{
//Return all signals in the recording.
//
//:rtype: list of :class:`H5Signal`
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  std::list<H5Signal> signals ;
  const std::string name = "/recording/signal" ;
  h5.iterateElems(name, NULL, saveSignal, (void *)&signals) ;
  return signals ;
  }


static herr_t saveClock(hid_t id, const char *name, void *op_data)
/*==============================================================*/
{
  SaveInfo *info = (SaveInfo *)op_data ;
  std::list<H5Clock> clk = reinterpret_cast<std::list<H5Clock> &>(info->listp) ;
  try {
    H5::DataSet dset = H5::DataSet(info->h5.openDataSet(name)) ;
    hobj_ref_t ref ;
    info->h5.reference(&ref, name) ;
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    H5::Attribute attr = dset.openAttribute("uri") ;
    std::string uri ;
    attr.read(varstr, uri) ;
    clk.push_back(H5Clock(uri, H5DataRef(dset, ref))) ;
    }
  catch (H5::FileIException e) { }
  return 0 ;
  }


std::list<H5Clock> H5Recording::clocks(void)
/*========================================*/
{
//Return all clocks in the recording.
//
//:rtype: list of :class:`H5Clock`
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  std::list<H5Clock> clocks ;
  const std::string name = "/recording/clock" ;
  h5.iterateElems(name, NULL, saveClock, (void *)&clocks) ;
  return clocks ;
  }


void H5Recording::storeMetadata(const std::string &metadata, const std::string &mimetype)
/*=====================================================================================*/
{
//Store metadata in the HDF5 recording.
//
//:param metadata: RDF serialised as a string.
//:type metadata: str or unicode
//:param mimetype: A mimetype string for the RDF format used.
//
//NOT C++:  Metadata is encoded as UTF-8 when stored. ****
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  try {
    h5.unlink("/metadata") ;
    }
  catch (H5::FileIException e) { }
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  H5::DataSpace scalar(H5S_SCALAR) ;
  H5::DataSet md = h5.createDataSet("/metadata", varstr, scalar) ;
  md.write(metadata, varstr, scalar) ;
  H5::Attribute attr = md.createAttribute("mimetype", varstr, scalar) ;
  attr.write(varstr, mimetype) ;
  md.close() ;
  h5.flush(H5F_SCOPE_GLOBAL) ;
  }


StringPair H5Recording::getMetadata(void)
/*=====================================*/
{
//:return: A 2-tuple of retrieved metadata and mimetype, or
//         (None, None) if the recording has no '/metadata' dataset.
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  try {
    H5::DataSet md = h5.openDataSet("/metadata") ;
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    H5::DataSpace scalar(H5S_SCALAR) ;
    std::string metadata ;
    md.read(metadata, varstr, scalar) ;
    H5::Attribute attr = md.openAttribute("mimetype") ;
    std::string mimetype ;
    attr.read(varstr, mimetype) ;
    return std::make_pair(std::string(metadata), std::string(mimetype)) ;
    }
  catch (H5::FileIException e) { }
  return StringPair() ;
  }
