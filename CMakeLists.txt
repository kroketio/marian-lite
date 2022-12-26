cmake_minimum_required(VERSION 3.17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(marian-lite VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)

# initializes ${CMAKE_INSTALL_*DIR} to sane defaults
include(GNUInstallDirs)

option(COMPILE_CPU "Compile CPU version" ON)
option(STATIC "Produce static binaries" OFF)
set(BUILD_ARCH native CACHE STRING "Compile for this CPU architecture.")

if(STATIC)
    message(STATUS "${PROJECT_NAME} - STATIC BUILD ON")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    set(YAMLCPP_USE_STATIC_LIBS ON)
    set(Protobuf_USE_STATIC_LIBS ON)
    set(BLA_STATIC ON)
endif()

include(CompilerStuff)

# system deps
find_package(YamlCpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(PCRE2 REQUIRED)
find_package(Protobuf REQUIRED)
#find_package(CLI11 REQUIRED)  # vendored for now...
# set(BLA_SIZEOF_INTEGER ...)  # TODO: maybe specifically force 32/64 bit
set(BLA_VENDOR "OpenBLAS")
find_package(BLAS REQUIRED)
find_package(ZLIB)

pkg_check_modules(SENTENCEPIECE REQUIRED sentencepiece)
find_library(PATHIE_LIBRARY NAMES pathie REQUIRED)
find_path(PATHIE_INCLUDE_DIR NAMES "pathie/pathie.hpp" REQUIRED)

add_subdirectory(src)
