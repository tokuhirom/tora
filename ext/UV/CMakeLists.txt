CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_BUILD_TYPE Release)

PROJECT(UV)
SET(serial "1.0.0")
SET(soserial "1")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/../../tora/"
    "${PROJECT_SOURCE_DIR}/../../vendor/libuv/include/"
)
ADD_LIBRARY(UV SHARED uv.cc)
set_target_properties(UV PROPERTIES PREFIX "")
set_target_properties(UV PROPERTIES OUTPUT_NAME "lib/UV_")

# test settings
ADD_CUSTOM_TARGET(test COMMAND prove --exec "${PROJECT_SOURCE_DIR}/../../bin/tora -I lib -I ../../lib" t/*.tra DEPENDS UV)
