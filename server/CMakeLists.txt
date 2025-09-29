include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1  # pick a stable release
)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use external fmt" FORCE)
FetchContent_MakeAvailable(spdlog)

add_executable(arbalesto_server
    src/main.cpp
    src/server.cpp
)

target_link_libraries(arbalesto_server
    PRIVATE
        arbalesto_shared   # already links SFML::Network
        spdlog::spdlog
)
