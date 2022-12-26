set(INTGEMM_DONT_BUILD_TESTS ON CACHE BOOL "Disable intgemm tests")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GCC 12.1 erroneously detects a bunch unitialised values
    if (CMAKE_CXX_COMPILER_VERSION MATCHES "^12.*")
        add_compile_options(-Wno-uninitialized -Wno-maybe-uninitialized)
    endif()
endif()

# Check if compiler supports AVX2 (this should only catch emscripten)
try_compile(INTGEMM_COMPILER_SUPPORTS_AVX2
        ${CMAKE_CURRENT_BINARY_DIR}/compile_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_test/avx2.cc)

# Check if compiler supports AVX512BW
try_compile(INTGEMM_COMPILER_SUPPORTS_AVX512BW
        ${CMAKE_CURRENT_BINARY_DIR}/compile_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_test/avx512bw.cc)

# Check if the compiler supports AVX512VNNI
try_compile(INTGEMM_COMPILER_SUPPORTS_AVX512VNNI
        ${CMAKE_CURRENT_BINARY_DIR}/compile_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_test/avx512vnni.cc)

if (NOT INTGEMM_COMPILER_SUPPORTS_AVX2 OR NOT INTGEMM_COMPILER_SUPPORTS_AVX512BW OR NOT INTGEMM_COMPILER_SUPPORTS_AVX512VNNI)
    set(UNSUPPORTED "Your compiler is too old to support")
    if (NOT INTGEMM_COMPILER_SUPPORTS_AVX2)
        set(UNSUPPORTED "${UNSUPPORTED} AVX2")
    endif()
    if (NOT INTGEMM_COMPILER_SUPPORTS_AVX512BW)
        set(UNSUPPORTED "${UNSUPPORTED} AVX512BW")
    endif()
    if (NOT INTGEMM_COMPILER_SUPPORTS_AVX512VNNI)
        set(UNSUPPORTED "${UNSUPPORTED} AVX512VNNI")
    endif()
    message(WARNING "${Orange}${UNSUPPORTED}.  Multiplication will be slower on CPUs that support these instructions. For details rerun cmake with --debug-trycompile then try to build in compile_tests/CMakeFiles/CMakeTmp.${ColourReset}")
endif()

configure_file(intgemm_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/intgemm_config.h)