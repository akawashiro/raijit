cmake_minimum_required(VERSION 3.4)
project(zydis-download NONE)

include(ExternalProject)
ExternalProject_Add(zydis
    GIT_REPOSITORY    https://github.com/zyantific/zydis.git
    GIT_TAG           v4.0.0
    SOURCE_DIR        "${CMAKE_CURRENT_BINARY_DIR}/zydis"
    BINARY_DIR        ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
    CMAKE_ARGS
    -DCMAKE_INSTALL_MESSAGE=LAZY
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    )
