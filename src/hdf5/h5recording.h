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

#ifndef _BSML_H5_RECORDING_
#define _BSML_H5_RECORDING_ 1

#include <vector>
#include <utility>

#include "hdf5/h5common.h"
#include "hdf5/h5signal.h"
#include "hdf5/h5clock.h"
#include "utility/utility.h"


namespace bsml {

  class H5Recording
  /*=============*/
  {
   private:

    std::string uri ;
    H5::H5File h5 ;

    H5Clock checkTiming(double, double, const std::string &, size_t) ;
    H5DataRef getDataRef(const std::string &, const std::string &) ;
    H5DataRef createDataset(const std::string &, int, hsize_t *, hsize_t *, void *,
      H5DataTypes, H5Compression) ;
    void setSignalAttributes(H5::DataSet, double, double, double, double,
      const std::string &, H5Clock) ;

    H5Signal createSignal(const std::string &, const std::string &,
      void *, size_t, H5DataTypes, std::vector<hsize_t>,
      double, double, double, double,
      const std::string &, const std::string &, H5Compression) ;

    std::list<H5Signal> createSignal(strlist, strlist,
      void *, size_t, H5DataTypes, double, double, double, double,
      const std::string &, const std::string &, H5Compression) ;

    H5Clock createClock(const std::string &, const std::string &,
      void *, size_t, H5DataTypes, double, double, H5Compression) ;


   public:
    H5Recording(const std::string &, H5::H5File) ;
    ~H5Recording(void) ;
    void close(void) ;

    std::string getUri(void) const { return uri ; } ;

    template <class T> H5Signal createSignal(const std::string &uri, const std::string &units,
    /*======================================================================================*/
     std::vector<T> data=std::vector<T>(),
     std::vector<hsize_t> shape=std::vector<hsize_t>(),
     double gain = 1.0,
     double offset = 0.0,
     double rate = 0.0,
     double period = 0.0,
     const std::string &clock = "",
     const std::string &timeunits = "",
     H5Compression compression = BSML_H5_DEFAULT_COMPRESSION)
    {
      T *dp = (T *)&data[0] ;
      return createSignal(uri, units, (void *)dp, data.size(), H5DataTypes(dp), shape,
                          gain, offset, rate, period, timeunits, clock, compression) ;
      }

    template <class T> std::list<H5Signal> createSignal(strlist uris, strlist units,
    /*============================================================================*/
     std::vector<T> data=std::vector<T>(),
     double gain = 1.0,
     double offset = 0.0,
     double rate = 0.0,
     double period = 0.0,
     const std::string &clock = "",
     const std::string &timeunits = "",
     H5Compression compression = BSML_H5_DEFAULT_COMPRESSION)
    {
      T *dp = (T *)&data[0] ;
      return createSignal(uris, units, (void *)dp, data.size(), H5DataTypes(dp),
                          gain, offset, rate, period, timeunits, clock, compression) ;
      }


    template <class T> H5Clock createClock(const std::string &uri, const std::string &units="",
    /*=======================================================================================*/
     std::vector<T> times=std::vector<T>(),
     double rate = 0.0,
     double period = 0.0,
     H5Compression compression = BSML_H5_DEFAULT_COMPRESSION)
    {
      T *tp = (T *)&times[0] ;
      return createClock(uri, units, (void *)tp, times.size(), H5DataTypes(tp),
                         rate, period, compression) ;
      }

    H5Signal getSignal(const std::string &) ;
    std::list<H5Signal> signals(void) ;

    H5Clock getClock(const std::string &) ;
    std::list<H5Clock> clocks(void) ;

    void storeMetadata(const std::string &, const std::string &) ;
    std::pair<std::string, std::string> getMetadata(void) ;

    } ;

  } ;

#endif
