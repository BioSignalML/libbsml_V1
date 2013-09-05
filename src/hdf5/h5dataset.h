/******************************************************************************
 *                                                                            *
 *  BioSignalML Management in C++                                             *
 *                                                                            *
 *  Copyright (c) 2010-2013  David Brooks                                     *
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

#ifndef _BSML_H5_DATASET_
#define _BSML_H5_DATASET_ 1

#include <string>
#include <H5Cpp.h>

/*!
 * An extensible array stored in a HDF5 dataset.
 *
 * The first dimension of an array is able to grow in size.
 */
class BSML::H5Dataset
/*=================*/
{

 private:
  hobj_ref_t reference ;

  int rank ;
  hsize_t *shape ;    // rowshape[rank]
  int rowsize ;       // Product(shape[n] | n > 0)
  hsize_t size ;      // shape[0]


 protected:

  std::string uri ;         //!< The URI for the dataset.
  H5::DataSet dataset ;     //!< The DataSet in the underlying HDF5 file.

  void extend(void *, size_t, H5::DataType, int, int) ;


 public:

  /*!
   * Default constructor.
   */
  H5Dataset():
  /*========*/
  dataset(H5::DataSet()), reference(0)
  {
    }

  /*!
   * Create a H5Dataset object from an existing dataset in a HDF5 file.
   *
   * \param ds a BSML::DataRef to the dataset.
   */
  H5Dataset(const BSML::H5DataRef &ds):
  /*=================================*/
  dataset(ds.first), reference(ds.second)
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
   * \param ds a BSML::DataRef to the dataset.
   */
  H5Dataset(const std::string &uri, const BSML::H5DataRef &ds):
  /*=========================================================*/
  uri(uri), dataset(ds.first), reference(ds.second)
  {
    H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
    H5::Attribute attr = dataset.createAttribute("uri", varstr, H5::DataSpace(H5S_SCALAR)) ;
    attr.write(varstr, uri) ;
    }

  /*!
   * Ensure the underlying dataset is closed.
   */
  ~H5Dataset()
  /*========*/
  {
    close() ;
    }

  /*!
   * Close the underlying dataset.
   */
  void close(void)
  /*============*/
  {
    dataset.close() ;
    }

  /*!
   * Get the DataSet object in the underlying HDF5 file.
   */
  H5::DataSet getDataset(void)
  /*========================*/
  {
    return dataset ;
    }

  /*!
   * Get a HDF5 reference to the DataSet object in the underlying HDF5 file.
   */
  hobj_ref_t getRef(void)
  /*===================*/
  {
    return reference ;
    }

  /*!
   * Get the number of elements in the first dimension of the dataset.
   */
  size_t length(void)
  /*===============*/
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
  std::string name(void)
  /*==================*/
  {
    int n = H5Iget_name(dataset.getId(), NULL, 0) ;
    if (n == 0) return std::string("") ;
    char name[n+1] ;
    H5Iget_name(dataset.getId(), name, n+1) ;
    return std::string(name) ;
    }

  } ;


#endif
