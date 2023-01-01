include(ExternalProject)
include(FetchContent)

set(THREADS "12")
message(STATUS "Vendoring libs for marian-lite")

FetchContent_Declare(intgemm
    GIT_REPOSITORY https://github.com/kroketio/intgemm.git
    GIT_TAG "b5da8f1d3d79f90a85332a293c69004d62e0406c"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(intgemm)

FetchContent_Declare(pathie
        GIT_REPOSITORY https://github.com/kroketio/pathie-cpp.git
        GIT_TAG "5a9732f5a8e2c3c1508a7aef526ae4c001c4f262"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        )
FetchContent_MakeAvailable(pathie)
FetchContent_GetProperties(pathie)

FetchContent_Declare(sentencepiece-browsermt
        GIT_REPOSITORY https://github.com/kroketio/sentencepiece-browsermt.git
        GIT_TAG "539cbc91704886c539286806eb2883f927000839"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        )
FetchContent_MakeAvailable(sentencepiece-browsermt)
