# Define project
cmake_minimum_required(VERSION 3.5)
project(Arbalesto VERSION 1 LANGUAGES C CXX)

# Default build
if(NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE "Release")
endif()

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
ExternalProject_Add(SFML 
		SOURCE_DIR  ${CMAKE_SOURCE_DIR}/ext/SFML 
    		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
ExternalProject_Get_Property(SFML INSTALL_DIR)
set(SFML_LIB ${INSTALL_DIR})

# ADD SOURCE FILES AND HEADERS OF EXECUTABLE
include_directories(${CMAKE_SOURCE_DIR}/include ${SFML_LIB}/include)

set(ARBALESTO_SRC
    ${CMAKE_SOURCE_DIR}/src/main.cpp
	${CMAKE_SOURCE_DIR}/src/game.cpp
	${CMAKE_SOURCE_DIR}/src/packets.cpp
	${CMAKE_SOURCE_DIR}/src/logger.cpp
	${CMAKE_SOURCE_DIR}/src/animation.cpp
	${CMAKE_SOURCE_DIR}/src/state.cpp
	${CMAKE_SOURCE_DIR}/src/game_state.cpp
	${CMAKE_SOURCE_DIR}/src/menu_state.cpp
	${CMAKE_SOURCE_DIR}/src/utils.cpp
	${CMAKE_SOURCE_DIR}/src/server/servernetwork.cpp
	${CMAKE_SOURCE_DIR}/src/client/clientnetwork.cpp
	
)

link_directories(${SFML_LIB}/lib ${TINY_XML_LIB}/lib)
add_executable(arbalesto ${ARBALESTO_SRC})

target_link_libraries(arbalesto 
					optimized sfml-system		debug sfml-system-d 
					optimized sfml-window		debug sfml-window-d 
					optimized sfml-graphics		debug sfml-graphics-d 
					optimized sfml-audio		debug sfml-audio-d
					optimized sfml-network		debug sfml-network-d 
					optimized tinyxml2		debug tinyxml2d) 