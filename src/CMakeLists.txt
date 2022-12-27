add_subdirectory(3rd_party)

file(GLOB_RECURSE SRCS *.cpp *.cc *.cp)  # rly
file(GLOB_RECURSE HDRS *.h *.hpp *.inl)

# filter some files from the glob recurse
list(FILTER SRCS EXCLUDE REGEX "SQLiteCpp/")
list(FILTER HDRS EXCLUDE REGEX "SQLiteCpp/")
list(FILTER SRCS EXCLUDE REGEX "proto/")
list(FILTER HDRS EXCLUDE REGEX "proto/")
list(FILTER SRCS EXCLUDE REGEX "faiss/")
list(FILTER HDRS EXCLUDE REGEX "faiss/")

# define CMake library target(s)
# compile both static, shared based on -DSTATIC, -DSHARED options
foreach(_BUILD_TYPE STATIC SHARED)
    if(NOT "${${_BUILD_TYPE}}")
        continue()
    endif()

    set(_TARGET "${PROJECT_NAME}-${_BUILD_TYPE}")
    add_library(${_TARGET} ${_BUILD_TYPE} ${SRCS} ${HDRS})
    set_target_properties(${_TARGET} PROPERTIES
            OUTPUT_NAME ${PROJECT_NAME}
            CXX_STANDARD 17
            VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}"
            SOVERSION "${CMAKE_PROJECT_VERSION_MAJOR}"
            )

    target_compile_definitions(${_TARGET} PRIVATE
            COMPILE_CPU=1
            USE_SENTENCEPIECE=1
            BLAS_FOUND=1
            _USE_TF_STRING_VIEW=1
            )

    target_link_libraries(${_TARGET} PRIVATE
            ${SQLite3_LIBRARIES}
            ZLIB::ZLIB
            sqlite3
            SQLiteCpp
            pthread
            dl
            BLAS::BLAS
            #        CLI11::CLI11
            proto
            ${PCRE2_LIBRARIES}
            ${SENTENCEPIECE_LIBRARIES}
            protobuf::libprotobuf-lite
            ${PATHIE_LIBRARY}
            ${YAMLCPP_LIBRARY}
            ${INTGEMM_LIBRARY}
            )

    target_link_libraries(${_TARGET} PUBLIC faiss proto)

    target_include_directories(${_TARGET} PUBLIC
            $<BUILD_INTERFACE:${ProtobufIncludePath}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party/proto>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party/faiss>
            $<INSTALL_INTERFACE:include>
            )

    target_include_directories(${_TARGET} PRIVATE
            ${YAMLCPP_INCLUDE_DIR}
            ${SQLite3_INCLUDE_DIRS}
            ${PCRE2_INCLUDE_DIRS}
            ${PATHIE_INCLUDE_DIR}
            ${INTGEMM_INCLUDE_DIR}
            ${SENTENCEPIECE_INCLUDE_DIRS}
            )

    install(TARGETS ${_TARGET}
            EXPORT ${PROJECT_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )

    list(APPEND _TARGETS "${_TARGET}")
endforeach()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marian-lite"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl"
        PATTERN "SQLiteCpp" EXCLUDE
        )

message(STATUS "=========================================== marian-lite: ${CMAKE_PROJECT_VERSION}")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC}")
message(STATUS "yaml-cpp: ${YAMLCPP_LIBRARY}")
message(STATUS "pcre2: ${PCRE2_LIBRARIES}")
message(STATUS "sqlite3 ${SQLite3_LIBRARIES}")
message(STATUS "BLAS: ${BLAS_LIBRARIES}")
message(STATUS "zlib: ${ZLIB_LIBRARIES}")
message(STATUS "sentencepiece: ${SENTENCEPIECE_LIBRARIES}")
message(STATUS "pathie: ${PATHIE_LIBRARY}")
message(STATUS "intgemm: ${INTGEMM_LIBRARY}")
message(STATUS "target(s): ${_TARGETS}")
message(STATUS "===========================================")
