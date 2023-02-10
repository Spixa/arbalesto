#include "game.hpp"
#include "server/servernetwork.hpp"
#include <thread>
#include <iostream>

int main() {
    std::string ans = log("launcher").prompt("Host a dedicated server or join a remote server or just host a server? (D/R/S)");

    char res = tolower(ans[0]);

    if (res == 'r') {
        std::string ip = log("launcher").prompt("Enter server IP");
        std::string portStr = log("launcher").prompt("Enter server port");
        std::string nickname = log("launcher").prompt("Enter nickname");

        log("launcher").info("Launching client");

        Game::getInstance()->run(nickname, ip, std::atoi(portStr.c_str()));
    } else if (res == 'd') {
        log("launcher").info("Running a local Arbalesto server so the debug client can join");
        Server s{37549};
        std::thread serverThr{&Server::run, &s};
        log("launcher").info("Launching client");

        Game::getInstance()->run("Player", "localhost", 37549);
    } else if (res == 's') {
        log("launcher").info("Starting local arbalesto server on port 37549");
        Server s{37549};

        s.run();
    } else {
        log("launcher").error("Bad response");
    }
}