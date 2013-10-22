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

#include "hdf5/bsml_h5.h"

using namespace bsml ;


H5Signal::H5Signal()
/*================*/
: H5Dataset(), index(-1)
{
  }


H5Signal::H5Signal(const std::string &uri, const H5DataRef &ds, int n)
/*==================================================================*/
: H5Dataset(ds), index(n)
{
  }


H5Signal::H5Signal(const std::string &uri, const H5DataRef &ds)
/*===========================================================*/
:H5Dataset(ds)
{
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  H5::Attribute attr = dataset.openAttribute("uri") ;
  int nsignals = attr.getSpace().getSimpleExtentNpoints() ;
  index = -1 ;
  if (nsignals == 1) {
    attr.read(varstr, this->uri) ;
    if (uri != this->uri) throw H5Exception("Corrupt URI reference for: " + uri) ;
    }
  else {
    char *uris[nsignals] ;
    attr.read(varstr, uris) ;
    int n = 0 ;
    while (n < nsignals) {
      if (index == -1 && strcmp(uri.c_str(), uris[n]) == 0) {
        this->uri = uri ;
        index = n ;
        }
      free(uris[n]) ;
      ++n ;
      }
    if (index == -1) throw H5Exception("Corrupt URI reference for: " + uri) ;
    }
  }


int H5Signal::signal_count(void)
/*============================*/
{
  H5::DataSpace uspace = dataset.openAttribute("uri").getSpace() ;
  int udims = uspace.getSimpleExtentNdims() ;
  if (udims == 0) return 1 ;  // SCALAR
  else {
    if (udims != 1) throw H5Exception("Dataset's 'uri' attribute has wrong shape: " + uri) ;
    return uspace.getSimpleExtentNpoints() ;
    }
  }


size_t H5Signal::clock_size(void)
/*=============================*/
{
  try {
    H5::Attribute attr = dataset.openAttribute("clock") ;
    hobj_ref_t ref ;
    attr.read(H5::PredType::STD_REF_OBJ, &ref) ;
    attr.close() ;
    H5::DataSet clk(H5Rdereference(H5Iget_file_id(dataset.getId()), H5R_OBJECT, &ref)) ;
    H5::DataSpace cspace = clk.getSpace() ;
    int cdims = cspace.getSimpleExtentNdims() ;
    hsize_t cshape[cdims] ;
    cspace.getSimpleExtentDims(cshape) ;
    return cshape[0] ;
    }
  catch (H5::AttributeIException e) { }
  return -1 ;
  }


void H5Signal::extend(void *data, size_t size, H5::DataType dtype)
/*==============================================================*/
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif

  H5Dataset::extend(data, size, dtype, signal_count(), clock_size()) ;
  }
