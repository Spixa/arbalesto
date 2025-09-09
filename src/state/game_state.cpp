#include "game_state.h"
#include "../game.h"

#include <random>
#include <iostream>
#include <magic_enum.hpp>

GameState::GameState() : State("game"), world{"overworld"} {
    world.addEntity(std::make_unique<ControllingPlayer>());

    sf::Vector2f center = {0, 0};
    int count = 90;
    float radius = 200;
    for (int i = 0; i < count; ++i) {
        float angle = 2.f * 3.14159265f * i / count; // angle in radians
        sf::Vector2f pos = center + sf::Vector2f{std::cos(angle) * radius, std::sin(angle) * radius};

        world.addEntity(std::make_unique<AiPlayer>(pos));
    }
}

GameState::~GameState() {

}

void GameState::start() {
    info("Started game state");
    view.setSize(Game::getInstance()->getWindow().getDefaultView().getSize());
    view.zoom(0.3);
}

void GameState::update(sf::Time dt) {
    world.update(dt);
    view.setCenter(world.getPlayer()->getPosition());

    if (tick_clock.getElapsedTime() >= sf::seconds(1 / TICKRATE)) {
        world.update_tick(dt);
        tick_clock.restart();
    }
}

void GameState::render(sf::RenderTarget& targ) {
    targ.draw(world);
}