#pragma once
#include "state.hpp"
#include "client/clientworld.hpp"

class Player;
class GameState : public State {
public:
    GameState();

    Player* getPlayer() { return p; }
    std::vector<Player*>* getPlayers() { return &players; }

    void addPlayer(std::string const& name, sf::Vector2f const& startingPosition);
public:
    void update(sf::Time deltaTime, ClientNetwork* client, sf::Clock& tickClock) override;
protected:
    void draw(sf::RenderTarget& targ, sf::RenderStates states) const override;
    Player* p;
    std::vector<Player*> players;
    ClientWorld world{};
};
