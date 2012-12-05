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

#ifndef _BSML_H5_DATASET_
#define _BSML_H5_DATASET_ 1

#include <string>
#include <H5Cpp.h>


class BSML::H5Dataset
/*=================*/
{

 private:
  hobj_ref_t reference ;

 protected:
  std::string uri ;
  H5::DataSet dataset ;

 public:
  H5Dataset() : dataset(H5::DataSet()), reference(0) { }
  H5Dataset(const std::string &uri, const BSML::H5DataRef &ds):
    uri(uri), dataset(ds.first), reference(ds.second) { }

  void close(void) { dataset.close() ; }

  H5::DataSet getDataset(void) { return dataset ; }
  hobj_ref_t getRef(void) { return reference ; }

  size_t length(void)           // Needs reworking....
  /*===============*/
  {
    if (dataset.getId() == 0) return 0 ;
    else
     dataset.getSpace().getSimpleExtentNpoints() ; // Only true for 1-D datasets
    }

  std::string name(void)
  /*==================*/
  {
    int n = H5Iget_name(dataset.getId(), NULL, 0) ;
    if (n == 0) return std::string("") ;
    char name[n+1] ;
    H5Iget_name(dataset.getId(), name, n+1) ;
    return std::string(name) ;
    }

  } ;


#endif
