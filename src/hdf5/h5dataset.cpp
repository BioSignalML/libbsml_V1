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

#include <stdlib.h>

#include "hdf5/bsml_h5.h"

using namespace bsml ;


/*!
 * Default constructor.
 */
H5Dataset::H5Dataset()
/*==================*/
: reference(0), dataset(H5::DataSet()), index_(-1)
{
  }

/*!
 * Create a H5Dataset object and initialise an existing dataset in a HDF5 file.
 *
 * \param uri the URI for the dataset.
 * \param ds a bsml::DataRef to the dataset.
 */
H5Dataset::H5Dataset(const std::string &uri, const H5DataRef &ds, int index)
/*========================================================================*/
: reference(ds.second), uri(uri), dataset(ds.first)
{
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  try {
    H5::Attribute attr = dataset.openAttribute("uri") ;
    int nsignals = attr.getSpace().getSimpleExtentNpoints() ;
    if (nsignals == 1) {
      std::string ds_uri = "" ;
      attr.read(varstr, ds_uri) ;
      if (uri != ds_uri) throw H5Exception("Dataset URI '" + ds_uri + "' should be '" + uri + "'") ;
      }
    else {
      char **uris = (char **)calloc(nsignals, sizeof(char *)) ;
      attr.read(varstr, uris) ;
      int ds_index = -1 ;
      int n = 0 ;
      while (n < nsignals) {
        if (ds_index == -1 && strcmp(uri.c_str(), uris[n]) == 0) {
          this->uri = uri ;
          ds_index = n ;
          }
        free(uris[n]) ;
        ++n ;
        }
      free(uris) ;

      if (ds_index == -1) throw H5Exception("Cannot find dataset for '" + uri + "'") ;
      else if (index < 0) index = ds_index ;
      else if (index != ds_index) throw H5Exception("Corrupt dataset reference for '" + uri + "'") ;
      }
    }
  catch (H5::AttributeIException e) {
    if (index < 0) {
      H5::Attribute attr = dataset.createAttribute("uri", varstr, H5::DataSpace(H5S_SCALAR)) ;
      attr.write(varstr, uri) ;
      }
    }
  index_ = index ;
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
H5::DataSet H5Dataset::get_dataset(void) const
/*------------------------------------------*/
{
  return dataset ;
  }

/*!
 * Get a HDF5 reference to the DataSet object in the underlying HDF5 file.
 */
hobj_ref_t H5Dataset::get_ref(void) const
/*-------------------------------------*/
{
  return reference ;
  }

/*!
 * Get the number of elements in the first dimension of the dataset.
 */
size_t H5Dataset::length(void) const
/*--------------------------------*/
{
  if (dataset.getId() == 0) return 0 ;
  else {
    H5::DataSpace dspace = dataset.getSpace() ;
    hsize_t *shape = (hsize_t *)calloc(dspace.getSimpleExtentNdims(), sizeof(hsize_t)) ;
    dspace.getSimpleExtentDims(shape) ;
    hsize_t result = shape[0] ;
    free(shape) ;
    return result ;
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
  char *name = (char *)calloc(n+1, sizeof(char)) ;
  H5Iget_name(dataset.getId(), name, n+1) ;
  std::string result(name) ;
  free(name) ;
  return result ;
  }


void H5Dataset::extend(void *data, size_t size, H5::DataType dtype, int nsignals, int clock_size)
/*=============================================================================================*/
{

  H5::DataSpace dspace = dataset.getSpace() ;
  int ndims = dspace.getSimpleExtentNdims() ;
  hsize_t
    *shape = (hsize_t *)calloc(ndims, sizeof(hsize_t)),
    *newshape = (hsize_t *)calloc(ndims, sizeof(hsize_t)),
    *count = (hsize_t *)calloc(ndims, sizeof(hsize_t)),
    *start = (hsize_t *)calloc(ndims, sizeof(hsize_t)) ;
  try {
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
  free(shape) ;
  free(newshape) ;
  free(count) ;
  free(start) ;
  }
