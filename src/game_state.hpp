#pragma once
#include "state.hpp"

class GameState : public State {
public:
    GameState() 
        : State("game", States::GameState)
    {

    }
public:
    void update(sf::Time deltaTime) override {

    }
protected:
    void draw(sf::RenderTarget& targ, sf::RenderStates states) const override {

    }
};
