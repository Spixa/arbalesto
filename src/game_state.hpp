#pragma once
#include "state.hpp"

class Player;
class GameState : public State {
public:
    GameState();
public:
    void update(sf::Time deltaTime, ClientNetwork* client, sf::Clock& tickClock) override;
protected:
    void draw(sf::RenderTarget& targ, sf::RenderStates states) const override;
    Player* p;
};
