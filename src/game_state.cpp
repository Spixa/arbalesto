#include "game_state.hpp"
#include "state_manager.hpp"
#include <chrono>
#include "player.hpp"

GameState::GameState() : State("game", States::GameStateType), p(new Player("Player", {50.f, 50.f}))
{
    L("Loaded game state");
    p->setControlled(true);

}

void GameState::update(sf::Time deltaTime, ClientNetwork* client, sf::Clock& tickClock) {
    p->update(deltaTime);

    for (auto x: players) {
        x->update(deltaTime);
    }

    if (p->isMoving() && tickClock.getElapsedTime().asMilliseconds() >= 50) {
        auto velo = p->getPosition();

        sf::Packet pos;
        pos << net::Packet::ClientMovementPacket << velo.x << velo.y;
        client->sendPacket(pos);

        tickClock.restart();
    }
}

void GameState::draw(sf::RenderTarget& targ, sf::RenderStates states) const {
    states.transform *= getTransform();

    targ.draw(*p, states);

    for (auto x: players) {
        targ.draw(*x, states);
    }
}

void GameState::addPlayer(std::string const& name, sf::Vector2f const& startingPosition) {
    players.push_back(new Player(name, startingPosition));
}