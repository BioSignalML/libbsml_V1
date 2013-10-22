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

#include <iostream>
#include <vector>

#include "bsml_h5.h"

using namespace bsml ;


int main(void)
{
  try {
    const std::string uri = "Test URI" ;
    const std::string file = "t2.h5" ;

    H5Recording h = H5create(uri, file, true) ;


    int testdata[] = { 100, 2, 3, 4, 5, 6 } ;
    std::vector<int> tv ;
    tv.assign(testdata, testdata+6) ;

    std::vector<hsize_t> dshape(2) ;
    dshape[0] = 2 ;
    dshape[1] = 3 ;


// ??? std::cout << h.createSignal<int>("signal 1", "mV", NULL, NULL, 1.0, 0.0, 1000.0) << std::endl ;
    H5Signal s = h.createSignal("signal 2", "mV", tv,  dshape, 1.0, 0.0, 2000.0) ;
    std::cout << s.name() << std::endl ;
    tv[0] += 1 ;
    s.extend(tv) ;
    tv[0] += 1 ;
    s.extend(tv) ;
    tv[0] += 1 ;
    s.extend(tv) ;
    tv[0] += 1 ;
    s.extend(tv) ;
   
//    h.createSignal<int>("signal 2", "mV") ;
    
//    h.createSignal<int>("signal 3", "mV", &tv, 360.0) ;

    H5Clock c = h.createClock("clock1", "s", tv) ;
    std::cout << c.name() << std::endl ;
    c.extend(tv) ;


    h.storeMetadata("metadata", "format") ;

    h.close() ;

    H5Recording r = H5open("t2.h5") ;
    std::cout << r.getUri() << std::endl ;
    r.close() ;
    }
  catch (H5Exception e) {
    std::cerr << "EXCEPTION: " << e.what() << "\n" ;
    }
  }
