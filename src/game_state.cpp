#include "game_state.hpp"
#include "state_manager.hpp"
#include "player.hpp"

GameState::GameState() : State("game", States::GameStateType), p(new Player())
{
    L("Loaded game state");
}

void GameState::update(sf::Time deltaTime, ClientNetwork* client) {
    p->update(deltaTime);

    // make timing a thing using global client-side tick mechanism
    if (p->isMoving() ) {
        auto velo = p->getVelocity();
    }
}

void GameState::draw(sf::RenderTarget& targ, sf::RenderStates states) const {
    states.transform *= getTransform();

    targ.draw(*p, states);
}