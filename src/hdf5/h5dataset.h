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


class BSML::H5Dataset
/*=================*/
{
 private:
  H5::DataSet dataset ;
  hobj_ref_t reference ;

 public:
  H5Dataset() : dataset(H5::DataSet()), reference(0) { }
  H5Dataset(const BSML::H5DataRef &ds) : dataset(ds.first), reference(ds.second) { }

  H5::DataSet getDataset(void) { return dataset ; }
  hobj_ref_t getRef(void) { return reference ; }

  size_t length(void) {           // Needs reworking....
    if (dataset.getId() == 0) return 0 ;
    else
     dataset.getSpace().getSimpleExtentNpoints() ; // Only true for 1-D datasets
    }
  } ;


#endif
