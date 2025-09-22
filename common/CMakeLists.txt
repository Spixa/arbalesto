add_library(arbalesto_shared
    src/packets.cpp
)

target_include_directories(arbalesto_shared
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(arbalesto_shared
    PUBLIC
        SFML::Network
        magic_enum::magic_enum
)
