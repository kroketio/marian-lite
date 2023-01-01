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
if(SHARED)
    add_library(marian-lite-SHARED SHARED ${SRCS} ${HDRS})
endif()
if(STATIC)
    add_library(marian-lite-STATIC STATIC ${SRCS} ${HDRS})
endif()

foreach(_BUILD_TYPE STATIC SHARED)
    if(NOT "${${_BUILD_TYPE}}")
        continue()
    endif()

    set(_TARGET "marian-lite-${_BUILD_TYPE}")

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
            SQLiteCpp
            ZLIB::ZLIB
            pthread
            dl
            BLAS::BLAS
            #        CLI11::CLI11
            proto
            ${PCRE2_LIBRARIES}
            protobuf::libprotobuf-lite
            ${YAMLCPP_LIBRARY}
            faiss
            )

    if(VENDORED_LIBS)
        target_link_libraries(${_TARGET} PRIVATE
                pathie
                intgemm
                sentencepiece-browsermt
                sentencepiece_train-browsermt
                )
    else()
        target_link_libraries(${_TARGET} PRIVATE
                sentencepiece-browsermt::sentencepiece-browsermt
                sentencepiece-browsermt::sentencepiece_train-browsermt
                pathie::pathie
                intgemm::intgemm
                )
    endif()

    target_include_directories(${_TARGET} PUBLIC
            $<BUILD_INTERFACE:${ProtobufIncludePath}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party>
            #$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party/proto>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party/faiss>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            )

    target_include_directories(${_TARGET} PRIVATE
            ${YAML_CPP_INCLUDE_DIR}
            ${PCRE2_INCLUDE_DIRS}
            ${ZLIB_INCLUDE_DIRS}
            )

    list(APPEND _TARGETS "${_TARGET}")
endforeach()

list(APPEND _TARGETS 
    proto
    faiss
    SQLiteCpp
)

install(TARGETS ${_TARGETS}
        EXPORT marian-liteTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/marian-lite"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl"
        PATTERN "SQLiteCpp" EXCLUDE
        )

install(EXPORT marian-liteTargets
        FILE marian-liteTargets.cmake
        NAMESPACE marian-lite::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/marian-lite
        )

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/../Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/marian-liteConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/marian-lite
        )

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/marian-liteConfigVersion.cmake"
        VERSION "${CMAKE_PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/marian-liteConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/marian-liteConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/marian-lite
        )

configure_file("${PROJECT_SOURCE_DIR}/marian-lite.pc.in" "marian-lite.pc" @ONLY)
if (NOT MSVC)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/marian-lite.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

# print some linked libs and their paths for informational purposes
message(STATUS "=========================================== marian-lite: ${CMAKE_PROJECT_VERSION}")
message(STATUS "SHARED: ${SHARED} | STATIC: ${STATIC} | VENDORED: ${VENDORED_LIBS}")
message(STATUS "yaml-cpp: ${YAML_CPP_LIBRARIES}")
message(STATUS "pcre2: ${PCRE2_LIBRARIES}")
message(STATUS "BLAS: ${BLAS_LIBRARIES}")
message(STATUS "zlib ${ZLIB_VERSION_STRING}: ${ZLIB_LIBRARIES}")

#get_property(INTGEMM_LIB_LOCATION_RELEASE TARGET intgemm::intgemm PROPERTY IMPORTED_LOCATION_RELEASE)
#get_property(INTGEMM_LIB_LOCATION_DEBUG TARGET intgemm::intgemm PROPERTY IMPORTED_LOCATION_DEBUG)
#get_property(INTGEMM_LIB_LOCATION TARGET intgemm::intgemm PROPERTY IMPORTED_LOCATION_NOCONFIG)
#message(STATUS "intgemm (r): ${INTGEMM_LIB_LOCATION_RELEASE} (d): ${INTGEMM_LIB_LOCATION_DEBUG} (n): ${INTGEMM_LIB_LOCATION}")
#
#get_property(SP_LIB_LOCATION_RELEASE TARGET sentencepiece-browsermt::sentencepiece-browsermt PROPERTY IMPORTED_LOCATION_RELEASE)
#get_property(SP_LIB_LOCATION_DEBUG TARGET sentencepiece-browsermt::sentencepiece-browsermt PROPERTY IMPORTED_LOCATION_DEBUG)
#get_property(SP_LIB_LOCATION TARGET sentencepiece-browsermt::sentencepiece-browsermt PROPERTY IMPORTED_LOCATION_NOCONFIG)
#message(STATUS "sentencepiece (r): ${SP_LIB_LOCATION_RELEASE} (d): ${SP_LIB_LOCATION_DEBUG} (n): ${SP_LIB_LOCATION}")
#
#get_property(PATHIE_LIB_LOCATION_RELEASE TARGET pathie::pathie PROPERTY IMPORTED_LOCATION_RELEASE)
#get_property(PATHIE_LIB_LOCATION_DEBUG TARGET pathie::pathie PROPERTY IMPORTED_LOCATION_DEBUG)
#get_property(PATHIE_LIB_LOCATION TARGET pathie::pathie PROPERTY IMPORTED_LOCATION_NOCONFIG)
#message(STATUS "pathie (r): ${INTGEMM_LIB_LOCATION_RELEASE} (d): ${INTGEMM_LIB_LOCATION_DEBUG} (n): ${INTGEMM_LIB_LOCATION}")

message(STATUS "target(s): ${_TARGETS}")
message(STATUS "===========================================")
