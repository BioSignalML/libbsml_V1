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
    bool closed ;


    H5DataRef get_dataref(const std::string &, const std::string &) ;
    H5Clock *check_timing(double, const std::string &, size_t) ;

    H5DataRef create_dataset(const std::string &, int, hsize_t *, hsize_t *, void *, H5DataTypes) ;

    void set_signal_attributes(const H5::DataSet &, double, double, double,
      const std::string &, const H5Clock *) ;

    H5Signal *create_signal(const std::string &, const Unit &,
      void *, size_t, H5DataTypes, std::vector<hsize_t>,
      double, double, double, H5Clock *) ;

    std::vector<H5Signal *> create_signal(const std::vector<std::string> &, const std::vector<Unit> &,
      void *, size_t, H5DataTypes,
      double, double, double, H5Clock *) ;

    H5Clock *create_clock(const std::string &, const Unit &, const std::vector<double> &, double) ;


   public:
    H5Recording(const std::string &, H5::H5File) ;
    ~H5Recording(void) ;

    static H5Recording *H5open(const std::string &, bool readonly=false) ;
    static H5Recording *H5create(const std::string &, const std::string &, bool replace=false) ;

    void close(void) ;


    template <class T> H5Signal *create_signal(const std::string &uri, const std::string &units,
    /*======================================================================================*/
     std::vector<T> data=std::vector<T>(),
     std::vector<hsize_t> shape=std::vector<hsize_t>(),
     double gain = 1.0,
     double offset = 0.0,
     double rate = 0.0,
     H5Clock *clock = nullptr)
    {
      T *dp = (T *)&data[0] ;
      return create_signal(uri, units, (void *)dp, data.size(), H5DataTypes(dp), shape,
                          gain, offset, rate, clock) ;
      }

    template <class T> std::list<H5Signal *> create_signal(const std::vector<std::string> &uris,
    /*-----------------------------------------------------------------------------*/
     const std::vector<Unit> &units,
     std::vector<T> data=std::vector<T>(),
     double gain = 1.0,
     double offset = 0.0,
     double rate = 0.0,
     H5Clock *clock = nullptr)
    {
      T *dp = (T *)&data[0] ;
      return create_signal(uris, units, (void *)dp, data.size(), H5DataTypes(dp),
                          gain, offset, rate, clock) ;
      }



    H5Signal *new_signal(const std::string &uri, const Unit &unit, double rate)
    /*---------------------------------------------------------------------*/
    {
      double *dp = nullptr ;
      H5Signal *signal = create_signal(uri, unit, nullptr, 0, H5DataTypes(dp),
                                       std::vector<hsize_t>(), 1.0, 0.0, rate, nullptr) ;
      this->add_signal(signal) ;
      return signal ;
      }

    H5Signal *new_signal(const std::string &uri, const Unit &unit, Clock *clock)
    /*------------------------------------------------------------------------*/
    {
      double *dp = nullptr ;
      H5Signal *signal = create_signal(uri, unit, nullptr, 0, H5DataTypes(dp), std::vector<hsize_t>(),
                                                           1.0, 0.0, 0.0, dynamic_cast<H5Clock *>(clock)) ;
      this->add_signal(signal) ;
      return signal ;
      }


    SignalGroup *signalgroup(const std::vector<std::string> &uris,
    /*----------------------------------------------------------*/
                             const std::vector<Unit> &units, double rate)
    {
      double *dp = nullptr ;
      std::vector<H5Signal *> sigs = create_signal(uris, units, nullptr, 0, H5DataTypes(dp),
                                                   1.0, 0.0, rate, nullptr) ;
      SignalGroup *signalgroup = new H5SignalGroup(uris.size()) ;
      for (size_t i = 0 ;  i < uris.size() ;  ++i) {
        this->add_signal(sigs[i]) ;
        signalgroup->set_signal(i, sigs[i]) ;
        }
      return signalgroup ;
      }

    SignalGroup *signalgroup(const std::vector<std::string> &uris,
    /*----------------------------------------------------------*/
                             const std::vector<Unit> &units, Clock *clock)
    {
      double *dp = nullptr ;
      std::vector<H5Signal *> sigs = create_signal(uris, units, nullptr, 0, H5DataTypes(dp),
                                                   1.0, 0.0, 0.0, dynamic_cast<H5Clock *>(clock)) ;
      SignalGroup *signalgroup = new H5SignalGroup(uris.size()) ;
      for (size_t i = 0 ;  i < uris.size() ;  ++i) {
        this->add_signal(sigs[i]) ;
        signalgroup->set_signal(i, sigs[i]) ;
        }
      return signalgroup ;
      }


    H5Clock *new_clock(const std::string &uri, const Unit &units, double rate)
    /*-------------------------------------------------------------------------------*/
    {
      H5Clock *clock = this->create_clock(uri, units, std::vector<double>(), rate) ;
      this->add_resource(clock) ;
      return clock ;
      }

    H5Clock *new_clock(const std::string &uri, const Unit &units, const std::vector<double> &times)
    /*-------------------------------------------------------------------------------------------*/
    {
      H5Clock *clock = this->create_clock(uri, units, times, 0.0) ;
      this->add_resource(clock) ;
      return clock ;
      }

#ifdef TODO_READ_HDF5
    H5Signal get_signal(const std::string &) ;
    std::list<H5Signal> get_signals(void) ;
#endif

    H5Clock *get_clock(const std::string &) ;
    std::list<H5Clock *> get_clocks(void) ;

    void store_metadata(const std::string &, const std::string &) ;
    std::pair<std::string, std::string> get_metadata(void) ;

    void save_metadata(void) ;
    } ;

  } ;

#endif
