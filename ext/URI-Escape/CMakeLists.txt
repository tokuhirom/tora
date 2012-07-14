CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wno-sign-compare -fstack-protector -fPIC -O2 -g")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_LINK_FLAGS_DEBUG "-Wl,-E' '-Wl,-Bsymbolic' '-rdynamic")
SET(CMAKE_BUILD_TYPE Release)

PROJECT(URI-Escape)
SET(serial "1.0.0")
SET(soserial "1")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/../../tora/")
ADD_LIBRARY(URI-Escape SHARED URI-Escape.cc)
set_target_properties(URI-Escape PROPERTIES PREFIX "")
set_target_properties(URI-Escape PROPERTIES OUTPUT_NAME "lib/URI/Escape")

# test settings
ADD_CUSTOM_TARGET(test COMMAND prove --exec "${PROJECT_SOURCE_DIR}/../../bin/tora -I lib -I ../../lib" t/*.tra DEPENDS URI-Escape)
