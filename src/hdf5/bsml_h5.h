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

#include <stdexcept>
#include <sstream>
#include <utility>
#include <string>
#include <list>

#include <H5Cpp.h>


#define to_string(N) (static_cast<std::ostringstream*>( &(std::ostringstream() << (N)) )->str())


#define BSML_H5_DEFAULT_DATATYPE    H5::PredType::IEEE_F64LE
#define BSML_H5_DEFAULT_COMPRESSION BSML::BSML_H5_COMPRESS_GZIP
#define BSML_H5_CHUNK_BYTES         (128*1024)


namespace BSML
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

  typedef std::list<std::string> StringList ;
  typedef std::pair<std::string, std::string> StringPair ;
  typedef std::pair<H5::DataSet, hobj_ref_t> H5DataRef ;

  H5Recording H5open(const std::string &, bool replace=false) ;
  H5Recording H5create(const std::string &, const std::string &, bool readonly=false) ;
  } ;


class BSML::H5Exception : public std::runtime_error
/*===============================================*/
{
 public:
  H5Exception(const std::string &msg) : std::runtime_error(msg) { }
  } ;


class BSML::H5DataTypes
/*===================*/
{
 public:
  H5::DataType mtype ;
  H5::DataType dtype ;

  template <class T> H5DataTypes(T *data) {
    throw H5Exception("Unsupported data type") ;
    }

  H5DataTypes(short *data): mtype(H5::PredType::NATIVE_SHORT), dtype(H5::PredType::STD_I16LE) { }

  H5DataTypes(int *data): mtype(H5::PredType::NATIVE_INT), dtype(H5::PredType::STD_I32LE) { }

  H5DataTypes(long *data): mtype(H5::PredType::NATIVE_LONG), dtype(H5::PredType::STD_I64LE) { }

  H5DataTypes(float *data): mtype(H5::PredType::NATIVE_FLOAT), dtype(H5::PredType::IEEE_F32LE) { }

  H5DataTypes(double *data): mtype(H5::PredType::NATIVE_DOUBLE), dtype(H5::PredType::IEEE_F64LE) { }

  H5DataTypes() : mtype(H5::PredType::NATIVE_DOUBLE), dtype(BSML_H5_DEFAULT_DATATYPE) { }
  H5DataTypes(long null) : mtype(H5::PredType::NATIVE_DOUBLE), dtype(BSML_H5_DEFAULT_DATATYPE) { }
  } ;



#include "h5recording.h"
#include "h5dataset.h"
#include "h5clock.h"
#include "h5signal.h"

#endif
