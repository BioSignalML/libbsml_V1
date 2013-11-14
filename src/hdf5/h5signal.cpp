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

#include <cstring>
#include <stdlib.h>

#include "hdf5/bsml_h5.h"
#include "model/signal.h"

using namespace bsml ;


H5Signal::H5Signal()
/*================*/
: Signal(), H5Dataset()
{
  }


H5Signal::H5Signal(const std::string &uri, const Unit &units, double rate, const H5DataRef &ds, int n)
/*--------------------------------------------------------------------------------------------------*/
: Signal(uri, units, rate), H5Dataset(uri, ds, n)
{
  }


H5Signal::H5Signal(const std::string &uri, const Unit &units, H5Clock *clock, const H5DataRef &ds, int n)
/*-----------------------------------------------------------------------------------------------------*/
: Signal(uri, units, clock), H5Dataset(uri, ds, n)
{
  }


H5Signal &H5Signal::operator=(const H5Signal &other)
/*------------------------------------------------*/
{
  Signal::operator=(other) ;
  H5Dataset::operator=(other) ;
  return *this ;
  }


int H5Signal::signal_count(void)
/*----------------------------*/
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
/*-----------------------------*/
{
  try {
    H5::Attribute attr = dataset.openAttribute("clock") ;
    hobj_ref_t ref ;
    attr.read(H5::PredType::STD_REF_OBJ, &ref) ;
    attr.close() ;
    H5::DataSet clk(H5Rdereference(H5Iget_file_id(dataset.getId()), H5R_OBJECT, &ref)) ;
    H5::DataSpace cspace = clk.getSpace() ;
    int cdims = cspace.getSimpleExtentNdims() ;
    hsize_t *cshape = (hsize_t *)calloc(cdims, sizeof(hsize_t)) ;
    cspace.getSimpleExtentDims(cshape) ;
    hsize_t result = cshape[0] ;
    free(cshape) ;
    return result ;
    }
  catch (H5::AttributeIException e) { }
  return -1 ;
  }


void H5Signal::extend(void *data, size_t size, H5::DataType dtype)
/*--------------------------------------------------------------*/
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  H5Dataset::extend(data, size, dtype, signal_count(), clock_size()) ;
  }
