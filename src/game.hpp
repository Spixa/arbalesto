#pragma once

#include <SFML/Graphics.hpp>

#include "resmanager.hpp"
#include "box.hpp"

class Player;
class Game : private sf::NonCopyable {
public:
    void run();
    static Game* getInstance();
    std::vector<Box*>& getBoxes() { return boxes; }
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
    std::vector<Box*> boxes;
    Player* p;

private: // Text
    sf::Text statistics_text_;
    sf::Font general_font_;

};