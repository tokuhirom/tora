CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
SET(CMAKE_CXX_FLAGS "-std=c++0x")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 -g")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_C_FLAGS_RELEASE "-Wall -O2 -g")
SET(CMAKE_C_FLAGS_DEBUG "-g")
SET(CMAKE_BUILD_TYPE Release)


PROJECT(Socket)
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/../../tora/")
ADD_LIBRARY(Socket SHARED socket.cc)
SET_TARGET_PROPERTIES(Socket PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(Socket PROPERTIES OUTPUT_NAME "lib/Socket")

# test settings
if (!TORA_VERSION)
    
endif(!TORA_VERSION)
message("tora: ${TORA_VERSION}")
ADD_CUSTOM_TARGET(test COMMAND prove --exec "${PROJECT_SOURCE_DIR}/../../bin/tora -I lib -I ../../lib" t/*.tra DEPENDS Socket)
INSTALL(TARGETS Socket LIBRARY DESTINATION "lib/tora-${TORA_VERSION}/")

