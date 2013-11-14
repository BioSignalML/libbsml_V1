cmake_minimum_required(VERSION 2.8.4)

project(BioSignalML CXX)

add_definitions(-DBSML_BUILD_LIBRARY)

include_directories(.)

IF (WIN32)
##  FIND_PACKAGE(HDF5 COMPONENTS CXX REQUIRED)  ## Need HDF5 1.8.12 to work ???
  SET (HDF5_INCLUDE_DIRS
    "c:/Program Files/HDF_Group/HDF5/1.8.11/include"
    "c:/Program Files/HDF_Group/HDF5/1.8.11/include/cpp"
    )
  SET (HDF5_LIBRARIES
    "c:/Program Files/HDF_Group/HDF5/1.8.11/lib/zlib.lib"
    "c:/Program Files/HDF_Group/HDF5/1.8.11/lib/szip.lib"
    "c:/Program Files/HDF_Group/HDF5/1.8.11/lib/hdf5.lib"
    "c:/Program Files/HDF_Group/HDF5/1.8.11/lib/hdf5_cpp.lib"
    )
ELSE()
  FIND_PACKAGE(HDF5 COMPONENTS CXX REQUIRED)
ENDIF()
INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIRS})


if (UNIX)
  add_definitions(--std=c++11)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

if (APPLE)
  add_definitions(--stdlib=libc++)
endif()

if (MSYS)
if (WIN32)
##From http://www.unknownroad.com/rtfm/VisualStudio/warningC4251.html
  add_definitions(/wd4251)
  add_definitions(/EHsc)

  include_directories(
    M:/build/boost_1_54_0
## For Raptor
    M:/build/win32/redland/include
    )
  link_directories(
#    "c:/Program Files/HDF_Group/HDF5/1.8.11/lib"
    M:/build/boost_1_54_0/bin.v2/libs/date_time/build/msvc-10.0/debug/link-static/threading-multi
    M:/build/win32/raptor/vc10/src
    )
endif()


set (SOURCES
  model/bsml.cpp
  model/object.cpp
  model/recording.cpp
  model/signal.cpp
  model/clock.cpp
  model/units.cpp
  model/datetime.cpp
  )

set (SOURCES ${SOURCES}
  hdf5/h5clock.cpp
  hdf5/h5dataset.cpp
  hdf5/h5recording.cpp
  hdf5/h5signal.cpp
  hdf5/h5common.cpp
  )

set (SOURCES ${SOURCES}
  utility/utility.cpp
  rdf/rdf.cpp
  rdf/rdfnames.cpp
  )
set (RDF_LIBRARIES
  raptor2
  )


if (APPLE)
  set (BOOST_LIBRARIES
    boost_date_time
    )
elseif (MSYS)       ## Static linking...
  set (RDF_LIBRARIES
    rdf
    rasqal
    raptor2
    xml2
    gmp
    pcre
    pthread
    curl
    idn
    iconv
    lzma
    gnutls
    nettle
    ssh2
    ws2_32
    )
  set (HDF5_LIBRARIES
    sz.a
    hdf5.a
    hdf5_cpp.a
    hdf5_hl_cpp.a
    )
  set (BOOST_LIBRARIES
    boost_date_time-mt.a
    )
endif()


add_library(biosignalml SHARED ${SOURCES})

target_link_libraries(biosignalml
  ${RDF_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${HDF5_LIBRARIES}
  )


# SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
# install(TARGETS biosignalml DESTINATION lib)
# install(FILES biosignalml.h DESTINATION include)
# install(FILES rdf/rdf.h rdf/rdfmap.h rdf/rdfnames.h DESTINATION include/rdf)

















