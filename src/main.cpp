#include "game.hpp"
#include "server/servernetwork.hpp"
#include <thread>

int main() {
    log("launcher").info("Running a local Arbalesto server so the debug client can join");
    Server s{37549};
    std::thread serverThr{&Server::run, &s};

    log("launcher").info("Launching client");
    Game::getInstance()->run();
}