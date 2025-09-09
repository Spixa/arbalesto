#include "game_state.h"
#include "../game.h"

#include <random>
#include <iostream>
#include <optional>
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
    view.zoom(1);
    // original = view.getSize();
}

void GameState::update(sf::Time dt) {
    static sf::Vector2f original;
    static float zoom;
    static sf::Time remaining;
    constexpr float ZOOM_TIME = 0.875;

    if (Game::getInstance()->isInitial()) {
        original = view.getSize();
        zoom = 4.0;
        remaining = sf::seconds(ZOOM_TIME);
    }

    if (remaining > sf::Time::Zero) {
        remaining -= dt;
        float t = 1.f - remaining.asSeconds() / ZOOM_TIME;
        if (t > 1.f) t = 1.f;

        // smooth interpolation: zoom goes from 3 -> 1
        zoom = 3.0f * (1.f - t) + 0.4f * t;
        view.setSize(original * zoom);
    }

    world.update(dt);
    view.setCenter(world.getPlayer()->getPosition());

    if (tick_clock.getElapsedTime() >= sf::seconds(1 / TICKRATE)) {
        world.update_tick(dt);
        tick_clock.restart();
    }
}

void GameState::update_event(const std::optional<sf::Event>& e) {
    if (!e.has_value()) return;  // optional is empty

    static sf::Vector2f original;
    static float zoom;
    if (Game::getInstance()->isInitial()) {
        original = view.getSize();
        zoom = 0.4;
    }

    // Check for MouseWheelScrolled safely
    if (const auto* scroll = e->getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->wheel == sf::Mouse::Wheel::Vertical) {
            zoom *= (scroll->delta > 0 ? 0.9f : 1.1f);
            view.setSize(original * zoom);
        }
    }
}

void GameState::render(sf::RenderTarget& targ) {
    targ.draw(world);
}