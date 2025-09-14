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
    int count = 50;
    float radius = 120;
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
        if (!msg.isEmpty()) {
            if (msg[0] != '/') {
                chat_box.push("&7[&dDemo&7] &6Player &2> &f" + msg);
            } else {
                chat_box.push("&cHey! &7Sorry, but that feature is unimplemented");
            }
        } else
            chat_box.push("&cHey! &7Sorry, but you can't send an empty message here");
    });
    chat_box.push("&6Welcome to &dArbalesto&6. You have &c10 &6seconds of grace!");
}

GameState::~GameState() {

}

void GameState::start() {
    info("Started game state");
    view.setSize(Game::getInstance()->getDefaultView().getSize());
    view.zoom(1.f);
}

void GameState::update(sf::Time dt) {
    auto* g = Game::getInstance();

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

    static sf::Vector2f original = view.getSize();
    static float zoom = 1;
    static bool ignore_next = false;
    auto* g = Game::getInstance();

    // Check for MouseWheelScrolled safely
    if (const auto* scroll = e->getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->wheel == sf::Mouse::Wheel::Vertical) {
            zoom *= (scroll->delta > 0 ? 0.9f : 1.1f);
            zoom = std::clamp(zoom, 0.1f, 0.7f);
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

        if (key->code == sf::Keyboard::Key::Slash && !chat_text.isFocused()) {
            chat_text.setFocused(true);
            g->pushFocus(UIWidget::CHAT);
            chat_box.setAlwaysVisible(true);
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

void GameState::tell(sf::String const& raw) {
    chat_box.push(raw);
}