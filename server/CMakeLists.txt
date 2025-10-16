include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1  # pick a stable release
)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "Use external fmt" FORCE)
FetchContent_MakeAvailable(spdlog)

option(ULTRA_RELEASE "Enable ultra-optimized release build" ON)

if(ULTRA_RELEASE)
  message(STATUS "Configuring in ULTRA_RELEASE mode (max performance)")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set(CMAKE_CXX_FLAGS_RELEASE
      "-O3 -DNDEBUG -flto -fno-plt -fomit-frame-pointer -static-libstdc++ -static-libgcc"
  )
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE
      "-s -flto"
  )
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()
  set(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -static-libstdc++ -static-libgcc"
  )
endif()

add_executable(arbalesto_server
    src/main.cpp
    src/server.cpp
    src/generator.cpp
)

target_link_libraries(arbalesto_server
    PRIVATE
        arbalesto_shared   # already links SFML::Network
        spdlog::spdlog
)
