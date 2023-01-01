add_subdirectory(faiss)

option(SQLITECPP_INTERNAL_SQLITE OFF)
add_subdirectory(SQLiteCpp EXCLUDE_FROM_ALL)

add_subdirectory(proto)