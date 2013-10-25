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

#include "model/recording.h"
#include "model/signal.h"
#include "model/clock.h"


namespace bsml {

  class H5Recording : public Recording
  /*================================*/
  {
   private:
    H5::H5File h5 ;

    H5Clock check_timing(double, const std::string &, size_t) ;

    H5DataRef get_dataref(const std::string &, const std::string &) ;

    H5DataRef create_dataset(const std::string &, int, hsize_t *, hsize_t *, void *, H5DataTypes) ;

    void set_signal_attributes(H5::DataSet, double, double, double,
      const std::string &, H5Clock) ;

    H5Signal create_signal(const std::string &, const std::string &,
      void *, size_t, H5DataTypes, std::vector<hsize_t>,
      double, double, double,
      const std::string &, const std::string &) ;

    std::list<H5Signal> create_signal(strlist, strlist,
      void *, size_t, H5DataTypes, double, double, double,
      const std::string &, const std::string &) ;

    H5Clock *create_clock(const std::string &, const Unit &, double *, size_t, double) ;


   public:
    H5Recording(const std::string &, H5::H5File) ;
    ~H5Recording(void) ;

    static H5Recording *H5open(const std::string &, bool readonly=false) ;
    static H5Recording *H5create(const std::string &, const std::string &, bool replace=false) ;

    void close(void) ;


    template <class T> H5Signal create_signal(const std::string &uri, const std::string &units,
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
      return create_signal(uri, units, (void *)dp, data.size(), H5DataTypes(dp), shape,
                          gain, offset, rate, period, timeunits, clock, compression) ;
      }

    template <class T> std::list<H5Signal> create_signal(strlist uris, strlist units,
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
      return create_signal(uris, units, (void *)dp, data.size(), H5DataTypes(dp),
                          gain, offset, rate, period, timeunits, clock, compression) ;
      }


    Clock *new_clock(const std::string &uri, const Unit &units, double rate)
    /*====================================================================*/
    {
      return create_clock(uri, units, NULL, 0, rate) ;
      }

    Clock *new_clock(const std::string &uri, const Unit &units,
    /*=======================================================*/
                              std::vector<double> times=std::vector<double>())
    {
      return create_clock(uri, units, &times[0], times.size(), 0.0) ;
      }

    H5Signal get_signal(const std::string &) ;
    std::list<H5Signal> get_signals(void) ;

    static H5Clock retrieve_clock(const std::string &uri, const H5DataRef &dataref) ;
    H5Clock get_clock(const std::string &) ;
    std::list<H5Clock> get_clocks(void) ;

    void store_metadata(const std::string &, const std::string &) ;
    std::pair<std::string, std::string> get_metadata(void) ;

    } ;

  } ;

#endif
