protobuf_generate_cpp(PROTO_SRC PROTO_HEADER sentencepiece.proto)
add_library(proto ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(proto PUBLIC ${PROTOBUF_LIBRARY})
TARGET_INCLUDE_DIRECTORIES(proto PUBLIC ${PROTOBUF_INCLUDE_DIR})

set(ProtobufIncludePath ${CMAKE_CURRENT_BINARY_DIR}
        CACHE INTERNAL "Path to generated protobuf files.")

message(STATUS "Compiling proto ... ${ProtobufIncludePath}")
