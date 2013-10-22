#ifndef _BSML_H5_COMMON
#define _BSML_H5_COMMON

#include <sstream>
#include <stdexcept>

#include <H5Cpp.h>


namespace bsml {

#define to_string(N) (static_cast<std::ostringstream*>( &(std::ostringstream() << (N)) )->str())

#define BSML_H5_DEFAULT_DATATYPE    H5::PredType::IEEE_F64LE
#define BSML_H5_DEFAULT_COMPRESSION BSML_H5_COMPRESS_GZIP
#define BSML_H5_CHUNK_BYTES         (128*1024)

  typedef std::pair<H5::DataSet, hobj_ref_t> H5DataRef ;

  class H5Exception : public std::runtime_error
  /*===============================================*/
  {
   public:
    H5Exception(const std::string &msg) ;
    } ;


  class H5DataTypes
  /*=============*/
  {
   public:
    H5::DataType mtype ;
    H5::DataType dtype ;

    template <class T> H5DataTypes(T *data) {
      throw H5Exception("Unsupported data type") ;
      }

    H5DataTypes(short *data) ;
    H5DataTypes(int *data) ;
    H5DataTypes(long *data) ;
    H5DataTypes(float *data) ;
    H5DataTypes(double *data) ;
    H5DataTypes() ;
    H5DataTypes(long null) ;
    } ;

  } ;

#endif
