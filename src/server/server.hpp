#pragma once

#include <string>
#include <vector>

#include "spec/player_spec.hpp"
#include "../logger.hpp"

#define strace(x) log("server").trace(x)
#define sinfo(x) log("server").info(x)
#define swarn(x) log("server").warn(x)
#define serror(x) log("server").error(x)

class LocalServer {
public:
    LocalServer() {
    
    }

    void join(server::Player const& player) {
        sinfo("New player joined the local world with name " + player.displayName + " at [" + std::to_string(player.xPos) + ", " +  std::to_string(player.yPos) + "]");
        players.push_back(player);
    }

    void move() {

    }

    void update() {
        
    }
private:
    std::vector<server::Player> players;
    sf::Clock timingClock;
};