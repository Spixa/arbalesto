#pragma once
#define L(x) log("main").trace(x)

#include <SFML/Graphics.hpp>

#include "resmanager.hpp"
#include "state_manager.hpp"
#include "client/clientnetwork.hpp"

class Player;
class Game : private sf::NonCopyable {
public:
    void run(std::string const& nickname, std::string const& ip, unsigned short port);
    static Game* getInstance();

    StateManager* getStateManager() { return &state_man_; }
    bool isFocused();

    unsigned int failedPacketCounter = 0;
private:
    Game();
    ~Game();
private:
    void processEvents(sf::Time elapsedTime);
    void update(sf::Time elapsedTime);
    void render();
    void updateStatistics(sf::Time elapsedTime);
private:

private:
    static Game* instance_;
    sf::RenderWindow window_;
    bool focused = false;
private: // Client
    ClientNetwork client;
private: // Text
    sf::Text statistics_text_;
    sf::Font general_font_;
    StateManager state_man_;

};