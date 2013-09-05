cmake_minimum_required(VERSION 2.8)

project(BioSignalML)

set (SOURCES "")
set (EXTRA_LIBS "")

include_directories(src)
add_subdirectory(src)

add_library(bsml ${SOURCES})
