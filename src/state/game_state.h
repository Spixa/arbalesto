#pragma once

#include "state.h"

#include "../entity/players.h"
#include "../world/world.h"
#include "../gui/textedit.h"

#define info(x) std::cout << "[client] INFO > " << x << std::endl
#define warn(x) std::cout << "[client] WARN > " << x << std::endl
#define error(x) std::cout << "[client] ERROR > " << x << std::endl

constexpr int TICKRATE = 20;

class GameState : public State {
public:
    GameState();
    virtual ~GameState();

public:
    void start() override;
    void update(sf::Time) override;
    void update_event(std::optional<sf::Event> const& e) override;
    void render(sf::RenderTarget&) override;
    void render_gui(sf::RenderTarget&) override;

    World* getWorld() { return &world; }
private:
    World world;
    TextEdit chat;
    sf::Clock tick_clock{};
};