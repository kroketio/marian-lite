FILE(GLOB FaissSources *.cpp *.h utils/*.cpp utils/*.h)
add_library(faiss OBJECT ${FaissSources})
target_compile_definitions(faiss PRIVATE FINTEGER=uint64_t)
target_include_directories(faiss PRIVATE ${CMAKE_CURRENT_LIST_DIR})
