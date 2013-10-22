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


using namespace bsml ;


H5Clock::H5Clock()
/*==============*/
: H5Dataset()
{
  }

H5Clock::H5Clock(const H5DataRef &ds)
/*=================================*/
: H5Dataset(ds)
{
  }

H5Clock::H5Clock(const std::string &uri, const H5DataRef &ds)
/*=========================================================*/
: H5Dataset(uri, ds)
{
  }

void H5Clock::extend(void *times, size_t size, H5::DataType dtype)
/*==============================================================*/
{
#if !H5_DEBUG
  H5::Exception::dontPrint() ;
#endif

  H5Dataset::extend(times, size, dtype, 1, -1) ;
  }
