#pragma once
#include "state.hpp"

class Player;
class GameState : public State {
public:
    GameState();
public:
    void update(sf::Time deltaTime, ClientNetwork* client) override;
protected:
    void draw(sf::RenderTarget& targ, sf::RenderStates states) const override;
    Player* p;
};
