cmake_minimum_required(VERSION 2.8)

project(BioSignalML)

if (APPLE)
  add_definitions(--std=c++11 --stdlib=libc++)
endif (APPLE)

include_directories(.)

set (SOURCES
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
  )


add_library(biosignalml SHARED ${SOURCES})

target_link_libraries(biosignalml
  c++
  rdf
  boost_date_time
  z
#  sz
#  hdf5
#  hdf5_cpp
#  hdf5_hl_cpp
  /usr/local/lib/libsz.a
  /usr/local/lib/libhdf5.a
  /usr/local/lib/libhdf5_cpp.a
  /usr/local/lib/libhdf5_hl_cpp.a
  )


# SET(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
# install(TARGETS biosignalml DESTINATION lib)
# install(FILES biosignalml.h DESTINATION include)
# install(FILES rdf/rdf.h rdf/rdfmap.h rdf/rdfnames.h DESTINATION include/rdf)

















