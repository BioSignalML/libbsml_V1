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
   std::vector<T> data=std::vector<T>(), std::vector<hsize_t> shape=std::vector<hsize_t>(),
   double gain=1.0, double offset=0.0, double rate=0.0, double period=0.0,
   const std::string &timeunits="", const std::string &clock="",
   BSML::H5Compression compression=BSML_H5_DEFAULT_COMPRESSION) {
//Create a dataset for a signal or group of signals in a HDF5 recording.
//
//:param uri: The URI(s) of the signal(s). If ``uri`` is an iterable then the dataset
//            is compound and contains several scalar signals.
//:param units: The units for the signal(s). Must be iterable and have the same number
//              of elements as ``uri`` when the dataset is compound.
//:param shape: The shape of a single data point. Must be None or scalar ('()') for
//              a compound dataset. Optional.
//:type shape: tuple
//:param data: Initial data for the signal(s). Optional.
//:type data: :class:`numpy.ndarray` or an iterable.
//:param dtype: The datatype in which to store data points. Must be specified if
//              no ``data`` is given.
//:type dtype: :class:`numpy.dtype`
//:param gain: If set, the signal's data values are multiplied by the ``gain`` when read. Optional.
//:type gain: float
//:param gain: If set, ``offset`` is subtracted from a data value before any gain
//             multiplication. Optional.
//:param rate: The frequency, as samples/time-unit, of data points.
//:type rate: float
//:param period: The time, in time-units, between data points.
//:type period: float
//:param timeunits: The units 'time' is measured in. Optional, default is seconds.
//:param clock: The URI of a clock dataset containing sample times. Optional.
//:return: The name of the signal dataset created.
//:rtype: str
//
//Only one of ``rate``, ``period``, or ``clock`` can be given.
    T *dp = (T *)&data[0] ;
    return createSignal(uri, units, (void *)dp, data.size(), BSML::H5DataTypes(dp), shape,
                        gain, offset, rate, period, timeunits, clock, compression) ;
    }

//  template <class T> std::string createSignal(const std::string &uri, const std::string &units,
//  /*=========================================================================================*/
//   std::vector<T> *data=NULL, double rate=0.0, double period=0.0, const std::string &timeunits="") {
//    return createSignal(uri, units, data, NULL, 1.0, 0.0, rate, period, timeunits) ;
//    }

  template <class T> std::list<BSML::H5Signal> createSignal(BSML::StringList uris, BSML::StringList units,
  /*====================================================================================================*/
   std::vector<T> data=std::vector<T>(), double gain=1.0, double offset=0.0,
   double rate=0.0, double period=0.0, const std::string &timeunits="", const std::string &clock="",
   BSML::H5Compression compression=BSML_H5_DEFAULT_COMPRESSION) {
    T *dp = (T *)&data[0] ;
    return createSignal(uris, units, (void *)dp, data.size(), BSML::H5DataTypes(dp),
                        gain, offset, rate, period, timeunits, clock, compression) ;
    }


  template <class T> BSML::H5Clock createClock(const std::string &uri, const std::string &units="",
  /*=============================================================================================*/
   std::vector<T> times=std::vector<T>(), double rate=0.0, double period=0.0,
   BSML::H5Compression compression=BSML_H5_DEFAULT_COMPRESSION) {
//Create a clock dataset in a HDF5 recording.
//
//:param uri: The URI for the clock.
//:param units: The units of the clock. Optional, default is seconds.
//:param shape: The shape of a single time point. Optional.
//:type shape: tuple
//:param times: Initial time points for the clock. Optional.
//:type times: :class:`numpy.ndarray` or an iterable.
//:param dtype: The datatype in which to store time points. Must be specified if
//              no ``times`` are given.
//:type dtype: :class:`numpy.dtype`
//:param rate (float): The sample rate of time points. Optional.
//:param period (float): The interval, in time units, between time points. Optional.
//:return: The name of the clock dataset created.
//:rtype: str
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
