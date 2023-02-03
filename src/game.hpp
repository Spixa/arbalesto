#pragma once

#include <SFML/Graphics.hpp>

#include "resmanager.hpp"
#include "state_manager.hpp"

class Player;
class Game : private sf::NonCopyable {
public:
    void run();
    static Game* getInstance();
private:
    Game();
    ~Game();
private:
    void processEvents();
    void update(sf::Time elapsedTime);
    void render();
    void updateStatistics(sf::Time elapsedTime);
private:

private:
    static Game* instance_;
    sf::RenderWindow window_;

private: // Map

private: // Text
    sf::Text statistics_text_;
    sf::Font general_font_;
    StateManager state_man_;

};