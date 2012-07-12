CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_BUILD_TYPE Release)

PROJECT(Path)
SET(serial "1.0.0")
SET(soserial "1")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/../../tora/")
ADD_LIBRARY(Path SHARED path.cc)
set_target_properties(Path PROPERTIES PREFIX "")
set_target_properties(Path PROPERTIES OUTPUT_NAME "lib/Path")

# test settings
ADD_CUSTOM_TARGET(test COMMAND prove --exec "${PROJECT_SOURCE_DIR}/../../bin/tora -I lib -I ../../lib" t/*.tra DEPENDS Path)
