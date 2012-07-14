CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
# SET(CMAKE_CXX_COMPILER "clang++")
# SET(CMAKE_C_COMPILER "clang")
SET(CMAKE_CC_FLAGS " ")
SET(CMAKE_CXX_FLAGS "-std=c++0x -D_REENTRANT -Wall -Wno-sign-compare -fstack-protector -fPIC")
SET(CMAKE_CXX_FLAGS_RELEASE " -O2 -g -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-E -Wl,-Bsymbolic -rdynamic -L.")
SET(CMAKE_BUILD_TYPE Release)

# operating specific settings
# see http://www.vtk.org/Wiki/CMake_Useful_Variables
if (WIN32)
    # win32 settings here
endif(WIN32)

PROJECT(tora CXX C)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

message(STATUS "module path: ${CMAKE_MODULE_PATH} ${CMAKE_ROOT} ${PROJECT_SOURCE_DIR}")

find_package(ICU REQUIRED)
if (ICU_VERSION VERSION_LESS "4.2")
    unset(ICU_FOUND CACHE)
    unset(ICU_INCLUDE_DIRS CACHE)
    unset(ICU_LIBRARIES CACHE)
    message(FATAL_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
endif ()

find_package(RE2C REQUIRED)

SET(TORA_VERSION_STR "0.0.5")
SET(TORA_OS          "${CMAKE_SYSTEM_NAME}")
SET(TORA_CC          "${CMAKE_C_COMPILER}")
SET(TORA_CXX         "${CMAKE_CXX_COMPILER}")
SET(TORA_CCFLAGS     "${CMAKE_CC_FLAGS}")
SET(TORA_CXXFLAGS    "${CMAKE_CXX_FLAGS}")
SET(TORA_PREFIX      "${CMAKE_INSTALL_PREFIX}")
SET(TORA_BUILD_TYPE  "${CMAKE_BUILD_TYPE}")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/tora/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/tora/config.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/lib/Config.tra.in ${CMAKE_CURRENT_BINARY_DIR}/lib/Config.tra)

RE2C_TARGET(
    tora/lexer.re
    tora/lexer.gen.cc
    ""
)
# generate files
ADD_CUSTOM_COMMAND(
    OUTPUT tora/vm.gen.cc tora/ops.gen.h tora/ops.gen.cc
    COMMAND perl -I misc/Text-MicroTemplate/ tora/vm.gen.pl
    DEPENDS tora/vm.gen.pl vm.inc
)
ADD_CUSTOM_COMMAND(
    OUTPUT tora/nodes.gen.h tora/nodes.gen.cc
    DEPENDS tora/nodes.gen.pl
    COMMAND perl tora/nodes.gen.pl > tora/nodes.gen.h
)
ADD_CUSTOM_COMMAND(
    OUTPUT tora/symbols.gen.cc tora/symbols.gen.h
    DEPENDS tora/symbols.gen.pl
    COMMAND perl -I misc/Text-MicroTemplate/ tora/symbols.gen.pl
)
ADD_EXECUTABLE(lemon tools/lemon/lemon.c)
ADD_CUSTOM_COMMAND(
    OUTPUT tora/parser.cc tora/parser.h
    DEPENDS lemon
    COMMAND ./lemon tora/parser.yy
)
ADD_CUSTOM_COMMAND(
    OUTPUT tora/token.gen.cc tora/token.gen.h
    DEPENDS tora/token.gen.pl tora/parser.h
    COMMAND perl tora/token.gen.pl
)
SET(TORA_SRCS
    tora/builtin.cc    tora/lexer.gen.cc  tora/op.cc        tora/peek.cc          tora/util.cc
    tora/compiler.cc         tora/operator.cc  tora/printf.cc        tora/value.cc
    tora/disasm.cc     tora/node.cc       tora/ops.gen.cc   tora/symbol_table.cc  tora/vm.cc
    tora/frame.cc      tora/nodes.gen.cc  tora/pad_list.cc  tora/symbols.gen.cc   tora/vm.gen.cc
    tora/inspector.cc  tora/object.cc     tora/parser.cc    tora/token.gen.cc

    tora/object/array.cc       tora/object/file.cc                tora/object/stat.cc    tora/value/exception.cc
    tora/object/bytes.cc       tora/object/file_package.cc        tora/object/str.cc     tora/value/hash.cc
    tora/object/caller.cc      tora/object/hash.cc                tora/object/symbol.cc  tora/value/int.cc
    tora/object/class.cc       tora/object/int.cc                 tora/object/time.cc    tora/value/object.cc
    tora/object/code.cc        tora/object/internals.cc           tora/value/array.cc    tora/value/range.cc
    tora/object/dir.cc         tora/object/metaclass.cc           tora/value/bool.cc     tora/value/regexp.cc
    tora/object/double.cc      tora/object/object.cc              tora/value/bytes.cc    tora/value/str.cc
    tora/object/dynaloader.cc  tora/object/re2_regexp_matched.cc  tora/value/class.cc
    tora/object/env.cc         tora/object/regexp.cc              tora/value/code.cc
)

set(libs re2 pthread dl icudata icuuc boost_system)

# libtora.a
ADD_LIBRARY(libtora_a ${TORA_SRCS})
SET_TARGET_PROPERTIES(libtora_a PROPERTIES OUTPUT_NAME "tora")
TARGET_LINK_LIBRARIES(libtora_a ${libs})

# libtora.so
#   ADD_LIBRARY(libtora_so SHARED ${TORA_SRCS})
#   SET_TARGET_PROPERTIES(libtora_so PROPERTIES OUTPUT_NAME "tora")
#   TARGET_LINK_LIBRARIES(libtora_so ${libs})

# bin/tora
ADD_EXECUTABLE(toraexe tora/main.cc)
TARGET_LINK_LIBRARIES(toraexe ${libs} libtora_a "boost_program_options")
SET_TARGET_PROPERTIES(toraexe PROPERTIES OUTPUT_NAME "bin/tora")
ADD_DEPENDENCIES(
    toraexe tora/ops.gen.h
)

#############################################################################
# test settings
#############################################################################
FILE(GLOB TEST_CC_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cc")
set(test_files)
# ADD_EXECUTABLE(tests/test_lexer.t tests/test_lexer.cc ${TORA_SRCS})
foreach(f ${TEST_CC_FILES})
    string(REPLACE ".cc" ".t" _output "${f}")
    message("test file: ${_output} ${f}")
    ADD_EXECUTABLE(
        "${_output}"
        "${f}"
    )
    TARGET_LINK_LIBRARIES("${_output}" ${libs} dl libtora_a)
endforeach(f)

# test settings
ADD_CUSTOM_TARGET(test COMMAND "${PERL_EXECUTABLE}" util/test.pl DEPENDS tora)

#############################################################################
# installation settings
#############################################################################
# INSTALL(TARGETS libtora_a ARCHIVE DESTINATION lib)
# INSTALL(TARGETS libtora_so LIBRARY DESTINATION lib)
INSTALL(TARGETS toraexe DESTINATION bin)

#############################################################################
# And libraries
#############################################################################
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/Socket/)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/Curl/)

