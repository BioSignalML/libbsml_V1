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
#include <stdlib.h>

#include <H5Cpp.h>

#include "hdf5/bsml_h5.h"
#include "rdf/rdf.h"


using namespace bsml ;

extern "C" {
  static herr_t save_signal(hid_t, const char *, void *) ;
  static herr_t save_clock(hid_t, const char *, void *) ;
  } ;

typedef struct {
  H5::H5File h5 ;
  void *listp ;
  } SaveInfo ;


H5Recording *H5Recording::H5create(const std::string &uri, const std::string &fname, bool replace)
/*==============================================================================================*/
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
    return new H5Recording(uri, h5) ;
    }
  catch (H5::FileIException e) {
  // Need to remove any file... (only if replace ??)
    throw H5Exception("Cannot create '" + fname + "': " + e.getDetailMsg()) ;
    }
  }


H5Recording *H5Recording::H5open(const std::string &fname, bool readonly)
/*=====================================================================*/
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
    char *uri = nullptr ;

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
      throw H5Exception("Internal error in BioSignalML HDF5 file: '" + fname + "'") ;
  
    return new H5Recording(std::string(uri), h5) ;
    }
  catch (H5::Exception e) {
    throw H5Exception("Invalid BioSignalML HDF5 file: '" + fname + "': " + e.getDetailMsg()) ;
    }
  }


H5Recording::H5Recording(const std::string &uri, H5::H5File h5)
/*===========================================================*/
: Recording(uri), h5(h5), closed(false) { }


H5Recording::~H5Recording(void)
/*===========================*/
{
  if (!closed) close() ;
  }

void H5Recording::close(void)
/*=========================*/
{
  h5.close() ;
  closed = true ;
  }



H5Clock *H5Recording::check_timing(double rate, const std::string &clock, size_t npoints)
/*-------------------------------------------------------------------------------------*/
{
  H5Clock *h5clock = nullptr ;
  if (rate != 0.0) {
    if (clock != "") throw H5Exception("Only one of 'rate' or 'clock' can be specified") ;
    }
  else if (clock != "") {
    h5clock = get_clock(clock) ;
    if (h5clock == nullptr)
      throw H5Exception("Clock doesn't exist") ;
    else if (h5clock->length() < npoints)
      throw H5Exception("Clock either doesn't have sufficient times") ;
    }
  else {
    throw H5Exception("No timing information given") ;
    }
  return h5clock ;
  }


H5DataRef H5Recording::create_dataset(const std::string &group, int rank, hsize_t *shape,
/*======================================================================================*/
 hsize_t *maxshape, void *data, H5DataTypes datatypes)
{
  H5Compression compression = BSML_H5_DEFAULT_COMPRESSION ;

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
  std::string dsetname = "/recording/" + group + "/" + std::to_string(grp.getNumObjs()) ;
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
    if (data != nullptr) dset.write(data, datatypes.mtype) ;
    hobj_ref_t reference ;
    h5.reference(&reference, dsetname) ;
    return H5DataRef(dset, reference) ;
    }
  catch (H5::FileIException e) {
    e.printError() ;
    throw H5Exception("Cannot create '" + group + "' dataset: " + e.getDetailMsg()) ;
    }
  }


void H5Recording::set_signal_attributes(const H5::DataSet &dset, double gain, double offset,
/*----------------------------------------------------------------------------------------*/
 double rate, const std::string &timeunits, const H5Clock *clock)
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
  else {
    attr = dset.createAttribute("clock", H5::PredType::STD_REF_OBJ, scalar) ;
    hobj_ref_t ref = clock->get_ref() ;
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


H5Signal *H5Recording::create_signal(const std::string &uri, const Unit &units,
/*--------------------------------------------------------------------------*/
 void *data=nullptr, size_t datasize=0, H5DataTypes datatypes=H5DataTypes(),
 std::vector<hsize_t> datashape=std::vector<hsize_t>(),
 double gain=1.0, double offset=0.0, double rate=0.0, H5Clock *clock=nullptr)
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  H5::Attribute attr ;
  H5::Group urigroup = h5.openGroup("/uris") ;
  try {
    attr = urigroup.openAttribute(uri) ;
    attr.close() ;
    throw H5Exception("Signal's URI '" + uri + "' already specified") ;
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
    npoints = (data != nullptr) ? datasize/elsize : 0 ;
    }
  else if (data != nullptr) {  // simple dataset, data determines shape
    npoints = datasize ;
    }
  shape[0] = npoints ;

  H5DataRef sigdata = create_dataset("signal", rank, shape, maxshape, data, datatypes) ;
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
  attr.write(varstr, units.as_string()) ;
  attr.close() ;

  set_signal_attributes(dset, gain, offset, rate, "", clock) ;
  h5.flush(H5F_SCOPE_GLOBAL) ;
  H5Signal *signal = (rate != 0.0) ? new H5Signal(uri, units, rate, sigdata, -1)
                                   : new H5Signal(uri, units, clock, sigdata, -1) ;
  return signal ;
  }


std::vector<H5Signal *> H5Recording::create_signal(const std::vector<std::string> &uris,
/*----------------------------------------------------------------------------------*/
 const std::vector<Unit> &units,
 void *data=nullptr, size_t datasize=0, H5DataTypes datatypes=H5DataTypes(),
 double gain=1.0, double offset=0.0, double rate=0.0, H5Clock *clock=nullptr)
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  if (uris.size() != units.size())
    throw H5Exception("'uri' and 'units' have different sizes") ;
  int nsignals = uris.size() ;
  std::vector<H5Signal *> signals(nsignals) ;

  if (nsignals == 1) {
    signals[0] = create_signal(uris[0], units[0],
                               data, datasize, datatypes, std::vector<hsize_t>(),
                               gain, offset, rate, clock) ;
    return signals ;
    }
  H5::Attribute attr ;
  H5::Group urigroup = h5.openGroup("/uris") ;
  int n ;

  for (n = 0 ;  n < nsignals ;  ++n) {
    try {
      attr = urigroup.openAttribute(uris[n]) ;
      attr.close() ;
      throw H5Exception("Signal's URI '" + uris[n] + "' already specified") ;
      }
    catch (H5::AttributeIException e) { }
    }
  size_t npoints = (data != nullptr) ? datasize/nsignals : 0 ;
  hsize_t maxshape[2] = { H5S_UNLIMITED, (hsize_t)nsignals } ;
  hsize_t shape[2]    = { npoints,       (hsize_t)nsignals } ;

  H5DataRef sigdata = create_dataset("signal", 2, shape, maxshape, data, datatypes) ;
  H5::DataSet dset = sigdata.first ;

  H5::DataSpace scalar(H5S_SCALAR) ;
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  hobj_ref_t reference = sigdata.second ;

  const char *values[nsignals] ;
  hsize_t dims[1] ;
  dims[0] = nsignals ;
  H5::DataSpace attrspace(1, dims, dims) ;

try {
  for (n = 0 ;  n < nsignals ;  ++n) {
    values[n] = uris[n].c_str() ;
    attr = urigroup.createAttribute(uris[n], H5::PredType::STD_REF_OBJ, scalar) ;
    attr.write(H5::PredType::STD_REF_OBJ, &reference) ;
    attr.close() ;
    signals[n] = (rate != 0.0) ? new H5Signal(uris[n], units[n], rate, sigdata, n)
                               : new H5Signal(uris[n], units[n], clock, sigdata, n) ;
    }
  attr = dset.createAttribute("uri", varstr, attrspace) ;
  attr.write(varstr, values) ;
  attr.close() ;
  attr = dset.createAttribute("units", varstr, attrspace) ;
  for (n = 0 ;  n < nsignals ;  ++n) values[n] = units[n].as_string().c_str() ;
  attr.write(varstr, values) ;
  attr.close() ;

  set_signal_attributes(dset, gain, offset, rate, "", clock) ;
  h5.flush(H5F_SCOPE_GLOBAL) ;

  return signals ;
  }


H5Clock *H5Recording::create_clock(const std::string &uri, const Unit &units,
/*------------------------------------------------------------------------*/
                          const std::vector<double> &times, double rate=0.0)
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  H5::Attribute attr ;
  H5::Group urigroup = h5.openGroup("/uris") ;
  try {
    attr = urigroup.openAttribute(uri) ;
    attr.close() ;
    throw H5Exception("Clock's URI '" + uri + "' already specified") ;
    }
  catch (H5::AttributeIException e) { }

  if (times.size() > 0 && rate != 0.0)
    throw H5Exception("A Clock cannot have both 'times' and a 'rate'") ;
  
  size_t npoints = 0 ;
  hsize_t maxshape[] = { H5S_UNLIMITED } ;
  hsize_t shape[]    = { times.size() } ;
  H5DataRef clkdata = create_dataset("clock", 1, shape, maxshape, (void *)&times[0], H5DataTypes(&rate)) ;
  H5::DataSet dset = clkdata.first ;

  H5::DataSpace scalar(H5S_SCALAR) ;
  hobj_ref_t reference = clkdata.second ;
  attr = urigroup.createAttribute(uri, H5::PredType::STD_REF_OBJ, scalar) ;
  attr.write(H5::PredType::STD_REF_OBJ, &reference) ;
  attr.close() ;

  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;

// This could go into H5Clock's constructor...
  attr = dset.createAttribute("units", varstr, scalar) ;
  attr.write(varstr, units.as_string()) ;
  attr.close() ;
  if (rate != 0.0) {
    attr = dset.createAttribute("rate", H5::PredType::IEEE_F64LE, scalar) ;
    attr.write(H5::PredType::NATIVE_DOUBLE, &rate) ;
    attr.close() ;
    }

  h5.flush(H5F_SCOPE_GLOBAL) ;
  return (rate != 0.0) ? new H5Clock(uri, units, rate, clkdata)
                       : new H5Clock(uri, units, times, clkdata) ;
  }


H5DataRef H5Recording::get_dataref(const std::string &uri, const std::string &prefix)
/*---------------------------------------------------------------------------------*/
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
    char *buf = (char *)malloc(len + 1) ;
    H5Rget_name(id, H5R_OBJECT, &ref, buf, len + 1) ;

    bool matched = (prefix.compare(0, std::string::npos, buf, prefix.size()) == 0) ;
    free(buf) ;
    if (matched) return H5DataRef(dset, ref) ;
    }
  catch (H5::AttributeIException e) { }
  return H5DataRef() ;
  }


H5Signal H5Recording::get_signal(const std::string &uri)
/*--------------------------------------------------=*/
{
//Find a signal from its URI.
//
//:param uri: The URI of the signal to get.
//:return: A :class:`H5Signal` containing the signal, or None if
//         the URI is unknown or the dataset is not that for a signal.
  H5DataRef dataref = get_dataref(uri, "/recording/signal/") ;
  if (dataref.first.getId() != 0) return H5Signal(uri, dataref, -1) ;
  throw H5Exception("Cannot find signal:" + uri) ;
  }


static herr_t save_signal(hid_t id, const char *name, void *op_data)
/*--------------------------------------------------------------=*/
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
      char *uris[nsignals] ;
      attr.read(varstr, uris) ;
      int n = 0 ;
      while (n < nsignals) {
        sig.push_back(H5Signal(std::string(uris[n]), dataref, n)) ;
        free(uris[n]) ;
        ++n ;
        }
      }
    }
  catch (H5::FileIException e) { }
  return 0 ;
  }


std::list<H5Signal> H5Recording::get_signals(void)
/*----------------------------------------------*/
{
//Return all signals in the recording.
//
//:rtype: list of :class:`H5Signal`
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  std::list<H5Signal> signals ;
  const std::string name = "/recording/signal" ;
  h5.iterateElems(name, nullptr, save_signal, (void *)&signals) ;
  return signals ;
  }


H5Clock *H5Recording::get_clock(const std::string &uri)
/*--------------------------------------------------*/
{
//Find a clock dataset from its URI.
//
//:param uri: The URI of the clock dataset to get.
//:return: A :class:`H5Clock` or None if the URI is unknown or
//         the dataset is not that for a clock.
  H5DataRef dataref = get_dataref(uri, "/recording/clock/") ;
  if (dataref.first.getId() != 0) return H5Clock::get_clock(uri, dataref) ;
  throw H5Exception("Cannot find clock:" + uri) ;
  }


static herr_t save_clock(hid_t id, const char *name, void *op_data)
/*--------------------------------------------------------------*/
{
  SaveInfo *info = (SaveInfo *)op_data ;
  std::list<H5Clock *> clk = reinterpret_cast<std::list<H5Clock *> &>(info->listp) ;
  try {
    H5::DataSet dset = H5::DataSet(info->h5.openDataSet(name)) ;

    hobj_ref_t ref ;
    info->h5.reference(&ref, name) ;
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    H5::Attribute attr = dset.openAttribute("uri") ;
    std::string uri ;
    attr.read(varstr, uri) ;
    clk.push_back(H5Clock::get_clock(uri, H5DataRef(dset, ref))) ;
    }
  catch (H5::FileIException e) { }
  return 0 ;
  }


std::list<H5Clock *> H5Recording::get_clocks(void)
/*--------------------------------------------*/
{
//Return all clocks in the recording.
//
//:rtype: list of :class:`H5Clock`
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  std::list<H5Clock *> clocks ;
  const std::string name = "/recording/clock" ;
  h5.iterateElems(name, nullptr, save_clock, (void *)&clocks) ;
  return clocks ;
  }


void H5Recording::store_metadata(const std::string &metadata, const std::string &mimetype)
/*------------------------------------------------------------------------------------=*/
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


std::pair<std::string, std::string> H5Recording::get_metadata(void)
/*--------------------------------------------------------------*/
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
  return std::make_pair("", "") ;
  }

void H5Recording::save_metadata(void)
/*---------------------------------*/
{
  this->store_metadata(this->serialise(rdf::Format::TURTLE).c_str(), rdf::Format::TURTLE) ;
  }
