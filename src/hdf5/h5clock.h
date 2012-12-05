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

#ifndef _BSML_H5_CLOCK_
#define _BSML_H5_CLOCK_ 1

#include "h5dataset.h"


class BSML::H5Clock : public BSML::H5Dataset
/*========================================*/
{
 private:

  void extend(void *, size_t, H5::DataType) ;


 public:

  H5Clock() : BSML::H5Dataset() { }
  H5Clock(const std::string &uri, const BSML::H5DataRef &ds) : BSML::H5Dataset(uri, ds) { }

  template <class T> void extend(std::vector<T> times)
  /*================================================*/
  {
//Extend a clock dataset in a HDF5 recording.
//
//:param uri: The URI of the clock dataset.
//:param times: Time points with which to extend the clock.
//:type times: :class:`numpy.ndarray` or an iterable.
    T *tp = (T *)&times[0] ;
    extend((void *)tp, times.size(), BSML::H5DataTypes(tp).mtype) ;
    }


  } ;


#endif
