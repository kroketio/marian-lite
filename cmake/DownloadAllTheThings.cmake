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

FetchContent_Declare(cli11
        GIT_REPOSITORY https://github.com/kroketio/cli11.git
        GIT_TAG "2.3.0"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        )
FetchContent_MakeAvailable(cli11)
FetchContent_GetProperties(cli11)

FetchContent_Declare(sentencepiece-browsermt
        GIT_REPOSITORY https://github.com/kroketio/sentencepiece-browsermt.git
        GIT_TAG "0.1.99"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        )
FetchContent_MakeAvailable(sentencepiece-browsermt)
FetchContent_GetProperties(sentencepiece-browsermt)
