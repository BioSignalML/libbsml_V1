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

#include "h5signal.h"
#include "h5clock.h"


class BSML::H5Recording
/*===================*/
{
 private:

  std::string uri ;
  H5::H5File h5 ;

  BSML::H5Clock checkTiming(double, double, const std::string &, size_t) ;
  BSML::H5DataRef getDataRef(const std::string &, const std::string &) ;
  BSML::H5DataRef createDataset(const std::string &, int, hsize_t *, hsize_t *, void *,
    BSML::H5DataTypes, BSML::H5Compression) ;
  void setSignalAttributes(H5::DataSet, double, double, double, double,
    const std::string &, H5Clock) ;

  BSML::H5Signal createSignal(const std::string &, const std::string &,
    void *, size_t, BSML::H5DataTypes, std::vector<hsize_t>,
    double, double, double, double,
    const std::string &, const std::string &, BSML::H5Compression) ;

  std::list<BSML::H5Signal> createSignal(StringList, StringList,
    void *, size_t, BSML::H5DataTypes, double, double, double, double,
    const std::string &, const std::string &, BSML::H5Compression) ;

  BSML::H5Clock createClock(const std::string &, const std::string &,
    void *, size_t, BSML::H5DataTypes, double, double, BSML::H5Compression) ;


 public:

  H5Recording(const std::string &, H5::H5File) ;
  ~H5Recording(void) ;
  void close(void) ;

  std::string getUri(void) const { return uri ; } ;

  template <class T> BSML::H5Signal createSignal(const std::string &uri, const std::string &units,
  /*============================================================================================*/

   std::vector<T> data=std::vector<T>(),
   std::vector<hsize_t> shape=std::vector<hsize_t>(),

   double gain=1.0,
   double offset=0.0,

   double rate=0.0,
   double period=0.0,
   const std::string &clock="",

   const std::string &timeunits="",

   BSML::H5Compression compression=BSML_H5_DEFAULT_COMPRESSION

                                                              ) {
    T *dp = (T *)&data[0] ;
    return createSignal(uri, units, (void *)dp, data.size(), BSML::H5DataTypes(dp), shape,
                        gain, offset, rate, period, timeunits, clock, compression) ;
    }

  template <class T> std::list<BSML::H5Signal> createSignal(BSML::StringList uris, BSML::StringList units,
  /*====================================================================================================*/

   std::vector<T> data=std::vector<T>(),

   double gain=1.0,
   double offset=0.0,

   double rate=0.0,
   double period=0.0,
   const std::string &clock="",

   const std::string &timeunits="",

   BSML::H5Compression compression=BSML_H5_DEFAULT_COMPRESSION

                                                              ) {
    T *dp = (T *)&data[0] ;
    return createSignal(uris, units, (void *)dp, data.size(), BSML::H5DataTypes(dp),
                        gain, offset, rate, period, timeunits, clock, compression) ;
    }


  template <class T> BSML::H5Clock createClock(const std::string &uri, const std::string &units="",
  /*=============================================================================================*/
   std::vector<T> times=std::vector<T>(),
   double rate=0.0,
   double period=0.0,
   BSML::H5Compression compression=BSML_H5_DEFAULT_COMPRESSION) {
    T *tp = (T *)&times[0] ;
    return createClock(uri, units, (void *)tp, times.size(), BSML::H5DataTypes(tp),
                       rate, period, compression) ;
    }

  BSML::H5Signal getSignal(const std::string &) ;
  std::list<BSML::H5Signal> signals(void) ;

  BSML::H5Clock getClock(const std::string &) ;
  std::list<BSML::H5Clock> clocks(void) ;

  void storeMetadata(const std::string &, const std::string &) ;
  std::pair<std::string, std::string> getMetadata(void) ;
  } ;


#endif
