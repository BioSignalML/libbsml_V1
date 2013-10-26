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

#include "hdf5/bsml_h5.h"
#include "model/clock.h"
#include "model/units.h"


using namespace bsml ;


H5Clock::H5Clock()
/*--------------*/
: Clock(), H5Dataset()
{
  }


H5Clock::H5Clock(const std::string &uri, const Unit &units, double rate,
/*--------------------------------------------------------------------*/
                                                            const H5DataRef &ds)
: Clock(uri, units, rate), H5Dataset(ds)
{
  }

H5Clock::H5Clock(const std::string &uri, const Unit &units, const std::vector<double> &times,
/*-----------------------------------------------------------------------------------------*/
                                                            const H5DataRef &ds)
: Clock(uri, units, times), H5Dataset(ds)
{
  }


H5Clock &H5Clock::operator=(const H5Clock &other)
/*---------------------------------------------*/
{
  Clock::operator=(other) ;
  H5Dataset::operator=(other) ;
  return *this ;
  }


void H5Clock::extend(void *times, size_t size, H5::DataType dtype)
/*--------------------------------------------------------------*/
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif
  H5Dataset::extend(times, size, dtype, 1, -1) ;
  }


H5Clock H5Clock::get_clock(const std::string &uri, const H5DataRef &dataref)
/*------------------------------------------------------------------------*/
{
  H5::DataSet dset = dataref.first ;
  H5::StrType varstr(H5::PredType::C_S1, H5T_VARIABLE) ;
  H5::Attribute attr = dset.openAttribute("units") ;
  std::string units ;
  attr.read(varstr, units) ;

  double rate = 0.0 ;
  try {
    attr = dataref.first.openAttribute("rate") ;
    attr.read(H5::PredType::NATIVE_DOUBLE, &rate) ;
    attr.close() ;
    }
  catch (H5::AttributeIException e) { }
  if (rate != 0.0) return H5Clock(uri, Unit(units), rate, dataref) ;

  size_t size = dset.getSpace().getSimpleExtentNpoints() ;
  std::vector<double> times(size) ;
  dset.read((void *)&times[0], H5DataTypes(&rate).mtype) ;
  return H5Clock(uri, Unit(units), times, dataref) ;
  }
