CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
SET(CMAKE_C_FLAGS_RELEASE "-Wall -O2")
SET(CMAKE_C_FLAGS_DEBUG "-g")
SET(CMAKE_BUILD_TYPE Release)


PROJECT(socket)
SET(serial "1.0.0")
SET(soserial "1")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/../../tora/")
ADD_LIBRARY(socket SHARED socket.cc)

# test settings
if (!TORA_VERSION)
    
endif(TORA_VERSION)
message("tora: ${TORA_VERSION}")
ADD_CUSTOM_TARGET(test COMMAND prove --exec "${PROJECT_SOURCE_DIR}/../../bin/tora -I lib -I ../../lib" t/*.tra DEPENDS socket)
INSTALL(TARGETS socket LIBRARY DESTINATION "lib/tora-${TORA_VERSION}/")

