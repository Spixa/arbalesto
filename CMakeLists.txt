cmake_minimum_required(VERSION 3.19)
project(Arbalesto VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Custom build option: ULTRA_RELEASE
# ---------------------------------------------------------
option(ULTRA_RELEASE "Enable ultra-optimized release build" OFF)

if(ULTRA_RELEASE)
  message(STATUS "Configuring in ULTRA_RELEASE mode (max performance)")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

  # Aggressive optimization
  set(CMAKE_CXX_FLAGS_RELEASE
      "-O3 -DNDEBUG -march=native -flto -fno-plt -fomit-frame-pointer -static-libstdc++ -static-libgcc"
  )
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE
      "-s -flto"
  )
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()

  # Normal release settings
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -static-libstdc++ -static-libgcc"
  )
endif()

include(FetchContent)

# -------------------------------
# Fetch SFML 3.0.1
# -------------------------------
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.1
)
FetchContent_MakeAvailable(SFML)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)

# -------------------------------
# Fetch magic_enum
# -------------------------------
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG v0.9.6
)
FetchContent_MakeAvailable(magic_enum)

# -------------------------------
# Target
# -------------------------------
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/game.cpp
  src/state/state_man.cpp
  src/state/game_state.cpp
  src/state/menu_state.cpp
  src/entity/entity.cpp
  src/entity/players.cpp
  src/entity/arrow.cpp
  src/entity/itemstack.cpp
  src/animation/sprite.cpp
  src/world/chunk.cpp
  src/world/world.cpp
  src/item/item.cpp
  src/gui/textedit.cpp
  src/gui/chatbox.cpp
  src/gui/warning.cpp
  src/gui/button.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    magic_enum::magic_enum
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${magic_enum_SOURCE_DIR}/include
)
