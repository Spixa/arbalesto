#include "game_state.h"
#include "../game.h"

#include <random>
#include <iostream>
#include <optional>
#include <magic_enum.hpp>

#include "../entity/itemstack.h"

GameState::GameState() : State("game"), world{"overworld"}, chat{"> ", Game::getInstance()->getFallbackFont(), 32} {

    world.addEntity(std::make_unique<ControllingPlayer>());

    sf::Vector2f center = {0, 0};
    int count = 90;
    float radius = 200;
    for (int i = 0; i < count; ++i) {
        float angle = 2.f * 3.14159265f * i / count; // angle in radians
        sf::Vector2f pos = center + sf::Vector2f{std::cos(angle) * radius, std::sin(angle) * radius};

        world.addEntity(std::make_unique<AiPlayer>(pos));
    }

    sf::FloatRect ui = Game::getInstance()->getUIBounds();

    float padding = 20.f;

    // bottom-left anchor
    chat.setPosition({
        ui.position.x + 5.f,
        ui.position.y + ui.size.y - chat.getLocalBounds().size.y - padding
    });

    chat.setSize({ui.size.x, 40.f});
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
    static bool ignore_next = false;
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

    if (const auto* key = e->getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::T && !chat.isFocused()) {
            chat.setFocused(true);
            Game::getInstance()->pushFocus(UIWidget::CHAT);
            ignore_next = true;
            return;
        }

        if (key->code == sf::Keyboard::Key::Escape && chat.isFocused()) {
            Game::getInstance()->popFocus(UIWidget::CHAT);
        }

        if (key->code == sf::Keyboard::Key::Enter && chat.isFocused()) {
            Game::getInstance()->popFocus(UIWidget::CHAT);
        }
    }

    if (const auto* text = e->getIf<sf::Event::TextEntered>()) {
        if (ignore_next) {
            ignore_next = false;
        } else if (chat.isFocused() && text->unicode < 128) {
            chat.input(static_cast<char>(text->unicode));
        }
    }
}

void GameState::render(sf::RenderTarget& targ) {
    targ.draw(world);
}

void GameState::render_gui(sf::RenderTarget& targ) {
    targ.draw(chat);
}