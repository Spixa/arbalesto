#include "game.hpp"
#include "server/servernetwork.hpp"
#include <thread>
#include <iostream>

int main() {
    log("launcher").info("Would you like to host a dedicated server or join a remote server? (D/R)");
    char ans;
    std::cin >> ans;

    char res = tolower(ans);

    if (res == 'r') {
        log("launcher").info("Launching client");
        Game::getInstance()->run();
    } else if (res == 'd') {
        log("launcher").info("Running a local Arbalesto server so the debug client can join");
        Server s{37549};
        std::thread serverThr{&Server::run, &s};
        log("launcher").info("Launching client");
        Game::getInstance()->run();
    } else {
        log("launcher").error("Bad response");
    }
}