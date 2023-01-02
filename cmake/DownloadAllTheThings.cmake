include(ExternalProject)
include(FetchContent)

set(THREADS "12")
message(STATUS "Vendoring libs for marian-lite")

FetchContent_Declare(intgemm
    GIT_REPOSITORY https://github.com/kroketio/intgemm.git
    GIT_TAG "0.0.3"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(intgemm)

FetchContent_Declare(pathie
        GIT_REPOSITORY https://github.com/kroketio/pathie-cpp.git
        GIT_TAG "0.1.3"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        )
FetchContent_MakeAvailable(pathie)
FetchContent_GetProperties(pathie)

FetchContent_Declare(sentencepiece-browsermt
        GIT_REPOSITORY https://github.com/kroketio/sentencepiece-browsermt.git
        GIT_TAG "0.1.96"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        )
FetchContent_MakeAvailable(sentencepiece-browsermt)
FetchContent_GetProperties(sentencepiece-browsermt)
