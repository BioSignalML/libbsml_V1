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

#ifndef _BSML_H5_CLOCK_
#define _BSML_H5_CLOCK_ 1

#include "hdf5/h5dataset.h"
#include "model/clock.h"
#include "model/units.h"


namespace bsml {

  class H5Clock : public Clock, public H5Dataset
  /*==========================================*/
  {
   private:
    void extend(void *, size_t, H5::DataType) ;

   public:
    H5Clock() ;
    H5Clock(const std::string &uri, const Unit &units, double rate, const H5DataRef &ds) ;
    H5Clock(const std::string &uri, const Unit &units, const std::vector<double> &times, const H5DataRef &ds) ;
    H5Clock &operator=(const H5Clock &other) ;

    static H5Clock *get_clock(const std::string &uri, const H5DataRef &ds) ;

    /*!
     * Extend a clock in its first dimension.
     *
     * \tparam T the numeric datatype of time elements.
     * \param data a vector of time elements. The shape of the underlying
     * dataset is used to find the number of successive elements that make
     * up a single array cell.
     */
    template <class T> void extend(std::vector<T> data)
    /*===============================================*/
    {
      T *dp = (T *)&data[0] ;
      extend((void *)dp, data.size(), H5DataTypes(dp).mtype) ;
      }

// Also get rate/period
// Constructors (dataset), (dataset, uri, rate, period)

// Store data by calling extend() after creating an empty dataset??
// Or or H5Dataset class to have a create() method? cf. H5Recording::createDataset()

    } ;

  } ;

#endif
