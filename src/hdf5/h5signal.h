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

#ifndef _BSML_H5_SIGNAL_
#define _BSML_H5_SIGNAL_ 1

#include "h5dataset.h"


class BSML::H5Signal : public BSML::H5Dataset
/*==========================================*/
{
 private:
  int index ;

  void extend(void *, size_t, H5::DataType) ;

 public:
  H5Signal(const std::string &uri, const BSML::H5DataRef &ds, int n) : H5Dataset(uri, ds), index(n) { }

  template <class T> void extend(std::vector<T> data)
  /*===============================================*/
  {
//Extend a signal dataset in a HDF5 recording.
//
//:param uri: The URI of the signal for a simple dataset, or of any
//            signal in a compound dataset.
//:param data: Data points for the signal(s).
//:type data: :class:`numpy.ndarray` or an iterable.
//
//If the dataset is compound (i.e. contains several signals) then the size of the
//supplied data must be a multiple of the number of signals.
    T *dp = (T *)&data[0] ;
    extend((void *)dp, data.size(), BSML::H5DataTypes(dp).mtype) ;
    }

  } ;


#endif
