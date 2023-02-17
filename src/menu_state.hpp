#pragma once
#include "state.hpp"

class MenuState : public State {
public:
    MenuState();
public:
    void update(sf::Time deltaTime, ClientNetwork* client) override;
protected:
    void draw(sf::RenderTarget& targ, sf::RenderStates states) const override;
};
