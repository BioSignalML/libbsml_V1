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

#include "bsml_h5.h"

using namespace BSML ;


void H5Clock::extend(void *times, size_t size, H5::DataType dtype)
/*==============================================================*/
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  H5::DataSpace dspace = dataset.getSpace() ;
  int ndims = dspace.getSimpleExtentNdims() ;
  if (ndims != 1) throw H5Exception("Clock not one-dimensional: " + uri) ;
  try {
    hsize_t shape[1], newshape[1] ;
    dspace.getSimpleExtentDims(shape) ;
    newshape[0] = shape[0] + size ;
    hsize_t count[] = { size } ;
    dataset.extend(newshape) ;
    dspace = dataset.getSpace() ;
    dspace.selectHyperslab(H5S_SELECT_SET, count, shape) ;
    H5::DataSpace mspace(1, count, count) ;
    dataset.write(times, dtype, mspace, dspace) ;
    }
  catch (H5::DataSetIException e) {
    throw H5Exception("Cannot extend dataset '" + uri + "': " + e.getDetailMsg()) ;
    }
  }

