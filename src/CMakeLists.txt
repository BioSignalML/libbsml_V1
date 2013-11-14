cmake_minimum_required(VERSION 2.8.4)

project(BioSignalML CXX)

include_directories(/usr/local/include)
include_directories(.)
link_directories(/usr/local/lib)
add_definitions(-DBSML_BUILD_LIBRARY)

add_definitions(--std=c++11)


#FIND_PACKAGE (HDF5 NAMES hdf5)
#INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})


if (APPLE)
  add_definitions(--stdlib=libc++)
endif()

if (MSYS)
  include_directories(
    ${CMAKE_FIND_ROOT_PATH}/include/rasqal
    ${CMAKE_FIND_ROOT_PATH}/include/raptor2
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
  set (HDF5_LIBRARIES
    sz
    hdf5
    hdf5_cpp
    )
  set (CPP_LIBRARIES
    c++
    )
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
  ${CPP_LIBRARIES}
  ${RDF_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${HDF5_LIBRARIES}
  z
  )


# SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
# install(TARGETS biosignalml DESTINATION lib)
# install(FILES biosignalml.h DESTINATION include)
# install(FILES rdf/rdf.h rdf/rdfmap.h rdf/rdfnames.h DESTINATION include/rdf)

















