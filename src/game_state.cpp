#include "game_state.hpp"
#include "state_manager.hpp"
#include "player.hpp"

GameState::GameState() : State("game", States::GameStateType), p(new Player())
{

}

void GameState::update(sf::Time deltaTime) {
    p->update(deltaTime);
}

void GameState::draw(sf::RenderTarget& targ, sf::RenderStates states) const {
    states.transform *= getTransform();

    targ.draw(*p, states);
}