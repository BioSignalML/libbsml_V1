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

#ifndef _BSML_H5_
#define _BSML_H5_ 1

#include <string>


namespace bsml
/*==========*/
{
  const std::string BSML_H5_MAJOR   = "1" ;
  const std::string BSML_H5_MINOR   = "0" ;
  const std::string BSML_H5_VERSION = std::string("BSML ") + BSML_H5_MAJOR + "." + BSML_H5_MINOR ;

  enum H5Compression {
    BSML_H5_COMPRESS_NONE,
    BSML_H5_COMPRESS_GZIP,
    BSML_H5_COMPRESS_SZIP
    } ;

  class H5Recording ;
  class H5Dataset ;
  class H5Clock ;
  class H5Signal ;
  class H5Exception ;
  class H5DataTypes ;

  H5Recording H5open(const std::string &, bool replace=false) ;
  H5Recording H5create(const std::string &, const std::string &, bool readonly=false) ;
  } ;

#include "hdf5/h5common.h"
#include "hdf5/h5recording.h"
#include "hdf5/h5dataset.h"
#include "hdf5/h5clock.h"
#include "hdf5/h5signal.h"

#endif
