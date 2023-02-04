#include "game.hpp"
#include "server/server.hpp"
#include <thread>

int main() {
    Server s{37549};
    
    std::thread serverThr{&Server::run, &s};

    Game::getInstance()->run();
}