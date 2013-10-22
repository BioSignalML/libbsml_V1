#include "h5common.h"

using namespace bsml ;


H5Exception::H5Exception(const std::string &msg)
/*--------------------------------------------*/
: std::runtime_error(msg)
{
  }


H5DataTypes::H5DataTypes(short *data)
/*---------------------------------*/
: mtype(H5::PredType::NATIVE_SHORT),
  dtype(H5::PredType::STD_I16LE)
{
  }

H5DataTypes::H5DataTypes(int *data)
/*-------------------------------*/
: mtype(H5::PredType::NATIVE_INT),
  dtype(H5::PredType::STD_I32LE)
{
  }

H5DataTypes::H5DataTypes(long *data)
/*--------------------------------*/
: mtype(H5::PredType::NATIVE_LONG),
  dtype(H5::PredType::STD_I64LE)
{
  }

H5DataTypes::H5DataTypes(float *data)
/*---------------------------------*/
: mtype(H5::PredType::NATIVE_FLOAT),
  dtype(H5::PredType::IEEE_F32LE)
{
  }

H5DataTypes::H5DataTypes(double *data)
/*----------------------------------*/
: mtype(H5::PredType::NATIVE_DOUBLE),
  dtype(H5::PredType::IEEE_F64LE)
{
  }

H5DataTypes::H5DataTypes()
/*----------------------*/
: mtype(H5::PredType::NATIVE_DOUBLE),
  dtype(BSML_H5_DEFAULT_DATATYPE)
{
  }

H5DataTypes::H5DataTypes(long null)
/*-------------------------------*/
: mtype(H5::PredType::NATIVE_DOUBLE),
  dtype(BSML_H5_DEFAULT_DATATYPE)
{
  }
