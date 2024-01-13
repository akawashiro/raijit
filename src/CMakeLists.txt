cmake_minimum_required(VERSION 3.16)
project(sloader)

enable_testing()
set(CMAKE_CXX_STANDARD 20)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# glog
include(third-party)
get_third_party(glog)
set(WITH_GFLAGS
    OFF
    CACHE INTERNAL "" FORCE)
set(WITH_UNWIND OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/glog
                 ${CMAKE_CURRENT_BINARY_DIR}/glog EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/glog
                    ${CMAKE_CURRENT_BINARY_DIR}/glog/src)
add_definitions(-DC10_USE_GLOG=1)

# zydis
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
option(ZYDIS_BUILD_SHARED_LIB "" OFF)
get_third_party(zydis)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/zydis
                 ${CMAKE_CURRENT_BINARY_DIR}/zydis EXCLUDE_FROM_ALL)

# cpython
SET(PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpython-install/include/python3.12)
SET(PYTHON_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/../cpython-install/lib/libpython3.12.so)

add_library(raijit SHARED raijit.cc write_insts.cc opcode_table.cc)
target_link_libraries(raijit glog::glog Zydis ${PYTHON_LIBRARY})
target_include_directories(raijit PRIVATE ${PYTHON_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glog ${CMAKE_CURRENT_BINARY_DIR}/glog/src)
set_target_properties(raijit PROPERTIES PREFIX "")
target_compile_options(
  raijit
  PRIVATE -shared
          -fPIC
          -Wall
          -Wextra
          -Wno-unused-parameter
          -O2
          -g)
