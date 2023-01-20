# at least g++ 5.0
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "FATAL ERROR: Compiling marian-lite requires at least g++ 5.0, your version is ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# Detect support CPU instrinsics for the current platform. This will
# only by used with BUILD_ARCH=native. For overridden BUILD_ARCH we
# minimally use -msse4.1. This seems to work with MKL.
set(INTRINSICS "")
list(APPEND INTRINSICS_NVCC)

if(BUILD_ARCH STREQUAL "native")
    message(STATUS "Checking support for CPU intrinsics")
    include(FindSSE)
    if(SSE2_FOUND)
        message(STATUS "SSE2 support found")
        set(INTRINSICS "${INTRINSICS} -msse2")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -msse2)
    endif(SSE2_FOUND)
    if(SSE3_FOUND)
        message(STATUS "SSE3 support found")
        set(INTRINSICS "${INTRINSICS} -msse3")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -msse3)
    endif(SSE3_FOUND)
    if(SSE4_1_FOUND)
        message(STATUS "SSE4.1 support found")
        set(INTRINSICS "${INTRINSICS} -msse4.1")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -msse4.1)
    endif(SSE4_1_FOUND)
    if(SSE4_2_FOUND)
        message(STATUS "SSE4.2 support found")
        set(INTRINSICS "${INTRINSICS} -msse4.2")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -msse4.2)
    endif(SSE4_2_FOUND)
    if(AVX_FOUND)
        message(STATUS "AVX support found")
        set(INTRINSICS "${INTRINSICS} -mavx")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -mavx)
    endif(AVX_FOUND)
    if(AVX2_FOUND)
        message(STATUS "AVX2 support found")
        set(INTRINSICS "${INTRINSICS} -mavx2")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -mavx2)
    endif(AVX2_FOUND)
    if(AVX512_FOUND)
        message(STATUS "AVX512 support found")
        set(INTRINSICS "${INTRINSICS} -mavx512f")
        list(APPEND INTRINSICS_NVCC -Xcompiler\ -mavx512f)
    endif(AVX512_FOUND)
else()
    set(INTRINSICS "-msse4.1")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
    # Clang-10.0.0 complains when CUDA is newer than 10.1
    set(CLANG_IGNORE_UNKNOWN_CUDA "-Wno-unknown-warning-option -Wno-unknown-cuda-version")
    set(CLANG_IGNORE_UNUSED_VALUES "-Wno-unused-value")
    set(CLANG_IGNORE_UNUSED_PRIVATE_FIELD -Wno-unused-private-field;) # This is necessary for the APPLE clang compiler to appear after -Werror
endif()
set(DISABLE_GLOBALLY "-Wno-unused-result ${CLANG_IGNORE_UNKNOWN_CUDA} ${CLANG_IGNORE_UNUSED_VALUES}") # This needs to appear here as well to appease clang11+ on linux

list(APPEND ALL_WARNINGS -Wall; -Wno-error; -Wextra; -Wno-unused-result; -Wno-deprecated;
        -Wno-pragmas; -Wno-unused-parameter; -Wno-unused-function;
        -Wno-unused-value; -Wno-unknown-pragmas; -Wno-sign-compare;
        -Wno-missing-field-initializers; ${CLANG_IGNORE_UNUSED_PRIVATE_FIELD})

# This warning does not exist prior to gcc 5.0
if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0)
    list(APPEND ALL_WARNINGS -Wsuggest-override -Wno-int-in-bool-context)
endif()

set(CMAKE_RDYNAMIC_FLAG "-rdynamic")
set(CMAKE_CXX_FLAGS                 "-std=c++17 -pthread ${CMAKE_GCC_FLAGS} -fPIC ${DISABLE_GLOBALLY} -march=${BUILD_ARCH} ${INTRINSICS} ${BUILD_WIDTH}")

if(ARM_FOUND)
    set(CMAKE_CXX_FLAGS_RELEASE       "-O0 ${CMAKE_RDYNAMIC_FLAG}")
    set(CMAKE_C_FLAGS_RELEASE         "-O0 ${CMAKE_RDYNAMIC_FLAG}")
else()
    set(CMAKE_CXX_FLAGS_RELEASE       "-O3 ${BUILD_WIDTH} -funroll-loops")
    set(CMAKE_C_FLAGS_RELEASE         "-O3 ${BUILD_WIDTH} -funroll-loops")
endif()

set(CMAKE_CXX_FLAGS_DEBUG           "-O0 -g ${CMAKE_RDYNAMIC_FLAG}")
set(CMAKE_CXX_FLAGS_SLIM            "-O3 ${BUILD_WIDTH} -funroll-loops -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELEASE} -g ${CMAKE_RDYNAMIC_FLAG}")
set(CMAKE_CXX_FLAGS_PROFILE         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_CXX_FLAGS_PROFGEN         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fprofile-generate -fprofile-correction")
set(CMAKE_CXX_FLAGS_PROFUSE         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fprofile-use -fprofile-correction")

# these need to be set separately
set(CMAKE_C_FLAGS                 "-pthread ${CMAKE_GCC_FLAGS} -fPIC ${DISABLE_GLOBALLY} -march=${BUILD_ARCH} ${INTRINSICS}")

set(CMAKE_C_FLAGS_DEBUG           "-O0 -g ${CMAKE_RDYNAMIC_FLAG}")
set(CMAKE_C_FLAGS_SLIM            "-O3 ${BUILD_WIDTH} -funroll-loops -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO  "${CMAKE_C_FLAGS_RELEASE} -g ${CMAKE_RDYNAMIC_FLAG}")
set(CMAKE_C_FLAGS_PROFILE         "${CMAKE_C_FLAGS_RELWITHDEBINFO} -pg")
set(CMAKE_C_FLAGS_PROFGEN         "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fprofile-generate -fprofile-correction")
set(CMAKE_C_FLAGS_PROFUSE         "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fprofile-use -fprofile-correction")

# with gcc 7.0 and above we need to mark fallthrough in switch case statements
# that can be done in comments for backcompat, but CCACHE removes comments.
# -C makes gcc keep comments.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -C")