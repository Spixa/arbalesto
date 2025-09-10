#include "game_state.h"
#include "../game.h"

#include <random>
#include <iostream>
#include <optional>
#include <magic_enum.hpp>

#include "../entity/itemstack.h"

GameState::GameState() : State("game"), world{"overworld"}, chat_text{"> ", Game::getInstance()->getFallbackFont(), 32}, chat_box{Game::getInstance()->getFallbackFont()} {

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
    chat_text.setPosition({
        ui.position.x + 5.f,
        ui.position.y + ui.size.y - chat_text.getLocalBounds().size.y - padding
    });

    chat_text.setSize({ui.size.x, 40.f});
    chat_text.setSubmitCallback([this](const sf::String& msg) {
        if (!msg.isEmpty())
            chat_box.push("&7[&dDemo&7] &6Player &2> &f" + msg);
        else
            chat_box.push("&cHey! &7Sorry, but you can't send an empty message here");
    });
}

GameState::~GameState() {

}

void GameState::start() {
    info("Started game state");
    view.setSize(Game::getInstance()->getWindow().getDefaultView().getSize());
    view.zoom(1);
}

void GameState::update(sf::Time dt) {
    static sf::Vector2f original;
    static float zoom;
    static sf::Time remaining;
    constexpr float ZOOM_TIME = 0.5;

    auto* g = Game::getInstance();

    if (g->isInitial()) {
        original = view.getSize();
        zoom = 4.0;
        remaining = sf::seconds(ZOOM_TIME);
    }

    if (remaining > sf::Time::Zero) {
        remaining -= dt;
        float t = 1.f - remaining.asSeconds() / ZOOM_TIME;
        if (t > 1.f) t = 1.f;

        // smooth interpolation: zoom goes from 3 -> 1
        zoom = 3.0f * (1.f - t) + 0.2f * t;
        view.setSize(original * zoom);
    }

    world.update(dt);
    chat_text.update();
    chat_box.update(dt, chat_text.isFocused());
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

    auto* g = Game::getInstance();

    if (g->isInitial()) {
        original = view.getSize();
        zoom = 0.4;
    }

    // Check for MouseWheelScrolled safely
    if (const auto* scroll = e->getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->wheel == sf::Mouse::Wheel::Vertical) {
            zoom *= (scroll->delta > 0 ? 0.9f : 1.1f);
            // zoom = std::clamp(zoom, 0.f, 3.f);
            view.setSize(original * zoom);
        }
    }

    if (const auto* key = e->getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::T && !chat_text.isFocused()) {
            chat_text.setFocused(true);
            g->pushFocus(UIWidget::CHAT);
            chat_box.setAlwaysVisible(true);
            ignore_next = true;
            return;
        }

        if (key->code == sf::Keyboard::Key::Escape && chat_text.isFocused()) {
            g->popFocus(UIWidget::CHAT);
            chat_box.setAlwaysVisible(false);
        }

        if (key->code == sf::Keyboard::Key::Enter && chat_text.isFocused()) {
            g->popFocus(UIWidget::CHAT);
            chat_box.setAlwaysVisible(false);
        }
    }

    if (const auto* text = e->getIf<sf::Event::TextEntered>()) {
        if (ignore_next) {
            ignore_next = false;
        } else if (chat_text.isFocused() && text->unicode < 128) {
            chat_text.input(static_cast<char>(text->unicode));
        }
    }
}

void GameState::render(sf::RenderTarget& targ) {
    targ.draw(world);
}

void GameState::render_gui(sf::RenderTarget& targ) {
    targ.draw(chat_text);
    targ.draw(chat_box);
}