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

namespace bsml {

  /*!
   * An extensible array stored in a HDF5 dataset.
   *
   * The first dimension of an array is able to grow in size.
   */
  class H5Dataset
  /*===========*/
  {

   private:
    hobj_ref_t reference ;
    int index_ ;

    int rank ;
    hsize_t *shape ;    // rowshape[rank]
    int rowsize ;       // Product(shape[n] | n > 0)
    hsize_t size ;      // shape[0]

   protected:
    std::string uri ;         //!< The URI for the dataset.
    H5::DataSet dataset ;     //!< The DataSet in the underlying HDF5 file.

   public:

    /*!
     * Default constructor.
     */
    H5Dataset() ;

    /*!
     * Create a H5Dataset object and initialise an existing dataset in a HDF5 file.
     *
     * \param uri the URI for the dataset.
     * \param ds a bsml::DataRef to the dataset.
     */
    H5Dataset(const std::string &uri, const H5DataRef &ds, int index) ;

    /*!
     * Ensure the underlying dataset is closed.
     */
    virtual ~H5Dataset() ;

    /*!
     * Close the underlying dataset.
     */
    void close(void) ;

    /*!
     * Get the DataSet object in the underlying HDF5 file.
     */
    H5::DataSet getDataset(void) ;

    /*!
     * Get a HDF5 reference to the DataSet object in the underlying HDF5 file.
     */
    hobj_ref_t getRef(void) ;

    /*!
     * Get the number of elements in the first dimension of the dataset.
     */
    size_t length(void) ;

    /*!
     * Get the name of the DataSet object in the underlying HDF5 file.
     */
    std::string name(void) ;


    void extend(void *, size_t, H5::DataType, int, int) ;

    } ;

  } ;

#endif
