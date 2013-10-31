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

#ifndef _BSML_H5_SIGNAL
#define _BSML_H5_SIGNAL

#include "hdf5/h5clock.h"
#include "hdf5/h5dataset.h"
#include "model/signal.h"


namespace bsml {

  class H5Signal : public Signal, public H5Dataset
  /*============================================*/
  {

   private:
//    double gain ;
//    double offset ;
    int signal_count(void) ;

    void extend(void *, size_t, H5::DataType) ;


   public:
    H5Signal() ;
    H5Signal(const std::string &uri, const Unit &units, double rate, const H5DataRef &ds, int n) ;
    H5Signal(const std::string &uri, const Unit &units, H5Clock *clock, const H5DataRef &ds, int n) ;
    H5Signal &operator=(const H5Signal &other) ;

    size_t clock_size(void) ;

    /*!
     * Extend a signal in its first dimension.
     *
     * \tparam T the numeric datatype of array elements.
     * \param data a vector of data elements. The shape of the underlying
     * dataset is used to find the number of successive elements that make
     * up a single array cell.
     */
    template <class T> void extend(std::vector<T> data)
    /*===============================================*/
    {
      T *dp = (T *)&data[0] ;
      extend((void *)dp, data.size(), H5DataTypes(dp).mtype) ;
      }

// Also get gain/offset/rate/period/clock
// Constructors (dataset), (dataset, uri, gain, offset, rate, period, clock)

    } ;


  class H5SignalGroup : public SignalGroup
  /*====================================*/
  {

   public:

    H5SignalGroup(size_t size)
    /*----------------------*/
    : SignalGroup(size)
    {
      }

    void extend(double *data, size_t size)
    /*----------------------------------*/
    {
      if (size > 0) {
        H5Signal *s0 = dynamic_cast<H5Signal *>((*this)[0]) ;
        dynamic_cast<H5Dataset *>(s0)->extend((void *)data, size, H5DataTypes(data).mtype,
                                              this->size(), s0->clock_size()) ;
        }
      }

    } ;


  } ;

#endif
