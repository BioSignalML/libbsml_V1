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


void H5Dataset::extend(void *data, size_t size, H5::DataType dtype, int nsignals, int clock_size)
/*=============================================================================================*/
{

  H5::DataSpace dspace = dataset.getSpace() ;
  int ndims = dspace.getSimpleExtentNdims() ;
  try {
    hsize_t shape[ndims], newshape[ndims], count[ndims], start[ndims] ;
    dspace.getSimpleExtentDims(shape) ;
    if (nsignals > 1) {         // compound dataset
      if (ndims != 2) throw H5Exception("Compound dataset has wrong shape: " + uri) ;
      start[1] = 0 ;
      count[0] = size/nsignals ;
      newshape[1] = count[1] = shape[1] ;
      }
    else {                      // simple dataset
      count[0] = size ;
      for (int n = 1 ;  n < ndims ;  ++n) {
        start[n] = 0 ;
        count[0] /= shape[n] ;
        newshape[n] = count[n] = shape[n] ;
        }
      }
    start[0] = shape[0] ;
    newshape[0] = shape[0] + count[0] ;

    if (clock_size >= 0 && clock_size < newshape[0])
      throw H5Exception("Clock for '" + uri + "' doesn't have sufficient times") ;






    dataset.extend(newshape) ;
    dspace = dataset.getSpace() ;
    dspace.selectHyperslab(H5S_SELECT_SET, count, start) ; // Starting at 'shape' for 'count'
    H5::DataSpace mspace(ndims, count, count) ;
    dataset.write(data, dtype, mspace, dspace) ;
    }
  catch (H5::DataSetIException e) {
    throw H5Exception("Cannot extend dataset '" + uri + "': " + e.getDetailMsg()) ;
    }
  }
