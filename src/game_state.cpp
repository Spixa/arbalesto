#include "game_state.hpp"
#include "state_manager.hpp"
#include "player.hpp"

GameState::GameState() : State("game", States::GameStateType), p(new Player())
{
    L("Loaded game state");
}

void GameState::update(sf::Time deltaTime, ClientNetwork* client, sf::Clock& tickClock) {
    p->update(deltaTime);

    if (p->isMoving() && tickClock.getElapsedTime().asMilliseconds() >= 50) {
        auto velo = p->getPosition();
        
        // L("--");
        // L("X: " + std::to_string(velo.x));
        // L("Y: " + std::to_string(velo.y));

        sf::Packet pos;
        pos << net::Packet::ClientMovementPacket << velo.x << velo.y;
        client->sendPacket(pos);

        tickClock.restart();
    }
}

void GameState::draw(sf::RenderTarget& targ, sf::RenderStates states) const {
    states.transform *= getTransform();

    targ.draw(*p, states);
}