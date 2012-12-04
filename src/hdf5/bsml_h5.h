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

#include <sstream>
#include <utility>
#include <string>
#include <list>

#include <H5Cpp.h>


#define to_string(N) (static_cast<std::ostringstream*>( &(std::ostringstream() << (N)) )->str())


namespace BSML
/*==========*/
{
  const std::string H5_MAJOR   = "1" ;
  const std::string H5_MINOR   = "0" ;
  const std::string BSML_H5_VERSION = std::string("BSML ") + H5_MAJOR + "." + H5_MINOR ;

  enum H5Compression {
    H5_COMPRESS_NONE,
    H5_COMPRESS_GZIP,
    H5_COMPRESS_SZIP
    } ;

  class H5Recording ;
  class H5Dataset ;
  class H5Clock ;
  class H5Signal ;
  class H5Exception ;
  typedef std::list<std::string> StringList ;
  typedef std::pair<H5::DataSet *, hobj_ref_t> H5DataRef ;
  typedef std::pair<H5::DataType, H5::DataType> H5DataTypes ;

  H5Recording H5open(const std::string &, bool) ;
  H5Recording H5create(const std::string &, const std::string &, bool) ;

  template <class T> H5DataTypes *H5dataTypes(T *data) {
    throw H5Exception("Unsupported data type") ;
    }
  H5DataTypes *H5dataTypes(short *data) {
    return new H5DataTypes(H5::PredType::NATIVE_SHORT, H5::PredType::STD_I16LE) ;
    }
  H5DataTypes *H5dataTypes(int *data) {
    return new H5DataTypes(H5::PredType::NATIVE_INT, H5::PredType::STD_I32LE) ;
    }
  H5DataTypes *H5dataTypes(long *data) {
    return new H5DataTypes(H5::PredType::NATIVE_LONG, H5::PredType::STD_I64LE) ;
    }
  H5DataTypes *H5dataTypes(float *data) {
    return new H5DataTypes(H5::PredType::NATIVE_FLOAT, H5::PredType::IEEE_F32LE) ;
    }
  H5DataTypes *H5dataTypes(double *data) {
    return new H5DataTypes(H5::PredType::NATIVE_DOUBLE, H5::PredType::IEEE_F64LE) ;
    }
  H5DataTypes *H5dataTypes(long null) {  // Default datatype
    return new H5DataTypes(H5::PredType::NATIVE_DOUBLE, H5::PredType::IEEE_F64LE) ;
    }

  } ;


#define H5_DEFAULT_COMPRESSION BSML::H5_COMPRESS_GZIP
#define H5_CHUNK_BYTES         (128*1024)


class BSML::H5Exception : public std::runtime_error
/*===============================================*/
{
 public:
  H5Exception(const std::string &msg) : std::runtime_error(msg) { }
  } ;


#include "h5recording.h"
#include "h5dataset.h"
#include "h5clock.h"
#include "h5signal.h"

#endif
