add_executable(arbalesto_server
    src/main.cpp
)

target_link_libraries(arbalesto_server
    PRIVATE
        arbalesto_shared   # already links SFML::Network
)
