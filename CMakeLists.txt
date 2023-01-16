cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(marian-lite
        VERSION 0.2.0
        DESCRIPTION "Compact version of browsermt/marian-dev"
        HOMEPAGE_URL https://github.com/kroketio/marian-lite
        LANGUAGES CXX C)

include(GNUInstallDirs)  # initializes ${CMAKE_INSTALL_*DIR} to sane defaults
include(CMakePackageConfigHelpers)

option(COMPILE_CPU "Compile CPU version" ON)
option(STATIC "Produce static binary" OFF)
option(SHARED "Produce shared binary" ON)
option(VENDORED_LIBS "auto-download and install dependencies automatically. Used during COMPILE_PYTHON. Discouraged in normal circumstances." OFF)
set(BUILD_ARCH native CACHE STRING "Compile for this CPU architecture.")

# internal options
option(USE_INTGEMM "intgemm" OFF)  # will be set to 'ON' when using x86_64
option(USE_RUY "Google Ruy" OFF)  # will be set to 'ON' when using ARM
option(USE_SIMD_UTILS "Enable simde to target instruction sets" OFF)  # will be set to 'ON' when using ARM

include(FindCcache)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm*")
    # using ARM
    include(cmake/armFlags.cmake)
    find_package(cpuinfo REQUIRED)
    set(USE_RUY TRUE)
    set(USE_SIMD_UTILS ON)

    add_compile_options(-fno-strict-aliasing -Wno-comment)

    if(APPLE)
        option(USE_RUY_SGEMM "Compile with Ruy SGEMM" OFF)
    else()
        option(USE_RUY_SGEMM "Compile with Ruy SGEMM" ON)
        add_compile_definitions(USE_RUY_SGEMM=1 RUY_PLATFORM_NEON=1)
    endif()

    set(USE_SIMD_UTILS ON)
    add_compile_definitions(ARM FMA SSE) #added for ARM

else()
    set(USE_INTGEMM TRUE)
    add_compile_definitions(USE_INTGEMM=1)
endif()

if(STATIC)
    message(STATUS "${PROJECT_NAME} - STATIC BUILD ON")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(YAMLCPP_USE_STATIC_LIBS ON)
    set(Protobuf_USE_STATIC_LIBS ON)
    set(BLA_STATIC ON)
endif()

include(CompilerStuff)

if(NOT VENDORED_LIBS)
    # https://github.com/kroketio/pathie-cpp/
    # https://github.com/kroketio/sentencepiece-browsermt/
    # https://github.com/kroketio/cli11/
    if(ARM_FOUND)
        # https://github.com/kroketio/ruy/
        find_package(ruy REQUIRED)
    else()
        # https://github.com/kroketio/intgemm/
        find_package(intgemm REQUIRED)
    endif()
    find_package(pathie REQUIRED)
    find_package(sentencepiece-browsermt REQUIRED)
    find_package(CLI11 REQUIRED)
else()
    include(cmake/DownloadAllTheThings.cmake)
endif()

# system deps
find_package(YamlCpp REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(PCRE2 REQUIRED)
find_package(Protobuf REQUIRED)
#find_package(CLI11 REQUIRED)  # vendored for now...
# set(BLA_SIZEOF_INTEGER ...)  # TODO: maybe specifically force 32/64 bit
set(BLA_VENDOR "OpenBLAS")
find_package(BLAS REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
