add_subdirectory(3rd_party)

file(GLOB_RECURSE SRCS *.cpp *.cc *.cp)  # rly
file(GLOB_RECURSE HDRS *.h *.hpp)

# filter some files from the glob recurse
list(FILTER SRCS EXCLUDE REGEX "SQLiteCpp/tests/")
list(FILTER HDRS EXCLUDE REGEX "SQLiteCpp/tests/")
list(FILTER SRCS EXCLUDE REGEX "SQLiteCpp/examples/")
list(FILTER HDRS EXCLUDE REGEX "SQLiteCpp/examples/")
list(FILTER SRCS EXCLUDE REGEX "sentencepiece/")
list(FILTER HDRS EXCLUDE REGEX "sentencepiece/")
list(FILTER SRCS EXCLUDE REGEX "faiss/")
list(FILTER HDRS EXCLUDE REGEX "faiss/")

add_library(marian-lite STATIC ${SRCS} ${HDRS})
target_compile_definitions(marian-lite PRIVATE COMPILE_CPU=1)
target_compile_definitions(marian-lite PRIVATE USE_SENTENCEPIECE=1)
target_compile_definitions(marian-lite PRIVATE BLAS_FOUND=1)
target_link_libraries(marian-lite PRIVATE
        ${YAMLCPP_LIBRARY}
        ${SQLite3_LIBRARIES}
        ZLIB::ZLIB
        sqlite3
        SQLiteCpp
        pthread
        dl
        BLAS::BLAS
#        CLI11::CLI11
        ${PCRE2_LIBRARIES}
        ${SENTENCEPIECE_LIBRARIES}
        )

target_link_libraries(marian-lite PUBLIC faiss pathie)

target_compile_definitions(marian-lite PRIVATE _USE_TF_STRING_VIEW=1)
target_include_directories(marian-lite PRIVATE ${SENTENCEPIECE_LIBRARY_DIRS})
set_target_properties(marian-lite PROPERTIES CXX_STANDARD 17)

target_include_directories(marian-lite PUBLIC
        $<BUILD_INTERFACE:${ProtobufIncludePath}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party/proto>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/3rd_party/faiss>
        $<INSTALL_INTERFACE:include>
        )

target_include_directories(marian-lite PRIVATE
        ${YAMLCPP_INCLUDE_DIR}
        ${SQLite3_INCLUDE_DIRS}
        ${PCRE2_INCLUDE_DIRS}
        ${PATHIE_INCLUDE_DIR}
        )

message(STATUS "=========================================== marian-lite: ${CMAKE_PROJECT_VERSION}")
message(STATUS "STATIC: ${STATIC}")
message(STATUS "yaml-cpp: ${YAMLCPP_LIBRARY}")
message(STATUS "pcre2: ${PCRE2_LIBRARIES}")
message(STATUS "sqlite3 ${SQLite3_LIBRARIES}")
message(STATUS "BLAS: ${BLAS_LIBRARIES}")
message(STATUS "zlib: ${ZLIB_LIBRARIES}")
message(STATUS "sentencepiece: ${SENTENCEPIECE_LIBRARIES}")
message(STATUS "===========================================")
