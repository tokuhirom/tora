CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_BUILD_TYPE Release)

PROJECT(curl)
SET(serial "1.0.0")
SET(soserial "1")
ADD_LIBRARY(curl SHARED curl.cc)
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/../../tora/")

# test settings
ADD_CUSTOM_TARGET(test COMMAND prove --exec "${PROJECT_SOURCE_DIR}/../../bin/tora -I lib -I ../../lib" t/*.tra DEPENDS curl)
