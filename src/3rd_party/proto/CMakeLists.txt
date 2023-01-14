protobuf_generate_cpp(PROTO_SRC PROTO_HEADER
        sentencepiece.proto
        sentencepiece_model.proto
        )

add_library(proto OBJECT ${PROTO_HEADER} ${PROTO_SRC})

target_link_libraries(proto PUBLIC protobuf::libprotobuf-lite)
target_include_directories(proto PRIVATE
        ${Protobuf_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR})

set(ProtobufIncludePath ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Path to generated protobuf files.")

install(FILES ${PROTO_HEADER} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marian-lite/3rd_party/proto/")
message(STATUS "Compiling proto ... ${ProtobufIncludePath}")
