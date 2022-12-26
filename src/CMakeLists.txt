add_subdirectory(3rd_party)

file(GLOB_RECURSE SRCS *.cpp *.cc *.cp)  # rly
file(GLOB_RECURSE HDRS *.h *.hpp)

# filter some files from the glob recurse
list(FILTER SRCS EXCLUDE REGEX "SQLiteCpp/")
list(FILTER HDRS EXCLUDE REGEX "SQLiteCpp/")
list(FILTER SRCS EXCLUDE REGEX "sentencepiece/")
list(FILTER HDRS EXCLUDE REGEX "sentencepiece/")
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
            CXX_STANDARD 17)

    target_compile_definitions(${_TARGET} PRIVATE
            COMPILE_CPU=1
            USE_SENTENCEPIECE=1
            BLAS_FOUND=1
            _USE_TF_STRING_VIEW=1
            )

    target_link_libraries(${_TARGET} PRIVATE
            ${YAMLCPP_LIBRARY}
            ${SQLite3_LIBRARIES}
            ZLIB::ZLIB
            sqlite3
            SQLiteCpp
            pthread
            dl
            BLAS::BLAS
            #        CLI11::CLI11
            proto
            protobuf::libprotobuf-lite
            ${PCRE2_LIBRARIES}
            ${SENTENCEPIECE_LIBRARIES}
            ${PATHIE_LIBRARY}
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
            ${SENTENCEPIECE_LIBRARY_DIRS}
            )

    list(APPEND _TARGETS "${_TARGET}")
endforeach()

message(STATUS "=========================================== marian-lite: ${CMAKE_PROJECT_VERSION}")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC}")
message(STATUS "yaml-cpp: ${YAMLCPP_LIBRARY}")
message(STATUS "pcre2: ${PCRE2_LIBRARIES}")
message(STATUS "sqlite3 ${SQLite3_LIBRARIES}")
message(STATUS "BLAS: ${BLAS_LIBRARIES}")
message(STATUS "zlib: ${ZLIB_LIBRARIES}")
message(STATUS "sentencepiece: ${SENTENCEPIECE_LIBRARIES}")
message(STATUS "target(s): ${_TARGETS}")
message(STATUS "===========================================")
