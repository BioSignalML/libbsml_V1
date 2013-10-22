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


using namespace bsml ;


/*!
 * Default constructor.
 */
H5Dataset::H5Dataset()
/*==================*/
: dataset(H5::DataSet()), reference(0)
{
  }

/*!
 * Create a H5Dataset object from an existing dataset in a HDF5 file.
 *
 * \param ds a bsml::DataRef to the dataset.
 */
H5Dataset::H5Dataset(const H5DataRef &ds)
/*=====================================*/
: dataset(ds.first), reference(ds.second)
{
  H5::Attribute attr = dataset.openAttribute("uri") ;
  if (attr.getSpace().getSimpleExtentNpoints() == 1) {
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    attr.read(varstr, uri) ;
    }
  }

/*!
 * Create a H5Dataset object and initialise an existing dataset in a HDF5 file.
 *
 * \param uri the URI for the dataset.
 * \param ds a bsml::DataRef to the dataset.
 */
H5Dataset::H5Dataset(const std::string &uri, const H5DataRef &ds)
/*=============================================================*/
: uri(uri), dataset(ds.first), reference(ds.second)
{
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  H5::Attribute attr = dataset.createAttribute("uri", varstr, H5::DataSpace(H5S_SCALAR)) ;
  attr.write(varstr, uri) ;
  }

/*!
 * Ensure the underlying dataset is closed.
 */
H5Dataset::~H5Dataset()
/*===================*/
{
  close() ;
  }

/*!
 * Close the underlying dataset.
 */
void H5Dataset::close(void)
/*=======================*/
{
  dataset.close() ;
  }

/*!
 * Get the DataSet object in the underlying HDF5 file.
 */
H5::DataSet H5Dataset::getDataset(void)
/*===================================*/
{
  return dataset ;
  }

/*!
 * Get a HDF5 reference to the DataSet object in the underlying HDF5 file.
 */
hobj_ref_t H5Dataset::getRef(void)
/*==============================*/
{
  return reference ;
  }

/*!
 * Get the number of elements in the first dimension of the dataset.
 */
size_t H5Dataset::length(void)
/*==========================*/
{
  if (dataset.getId() == 0) return 0 ;
  else {
    H5::DataSpace dspace = dataset.getSpace() ;
    hsize_t shape[dspace.getSimpleExtentNdims()] ;
    dspace.getSimpleExtentDims(shape) ;
    return shape[0] ;
    }
  }

/*!
 * Get the name of the DataSet object in the underlying HDF5 file.
 */
std::string H5Dataset::name(void)
/*=============================*/
{
  int n = H5Iget_name(dataset.getId(), NULL, 0) ;
  if (n == 0) return std::string("") ;
  char name[n+1] ;
  H5Iget_name(dataset.getId(), name, n+1) ;
  return std::string(name) ;
  }


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
