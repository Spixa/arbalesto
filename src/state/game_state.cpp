#include "game_state.h"
#include "../game.h"

#include <random>
#include <iostream>
#include <optional>
#include <magic_enum.hpp>

#include "../entity/itemstack.h"

GameState::GameState() : State("game"), world{"overworld"}, chat_text{"> ", Game::getInstance()->getFallbackFont(), 23}, chat_box{Game::getInstance()->getFallbackFont()} {

    world.addEntity(std::make_unique<ControllingPlayer>());
    sf::FloatRect ui = Game::getInstance()->getUIBounds();

    float padding = 20.f;

    // bottom-left anchor
    chat_text.setPosition({
        ui.position.x + 5.f,
        ui.position.y + ui.size.y - chat_text.getLocalBounds().size.y - padding
    });

    chat_text.setSize({ui.size.x, 25.f});
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
    original = view.getSize();
    zoom = 0.4f;
    view.setSize(original * zoom);
}

void GameState::update(sf::Time dt) {
    auto* g = Game::getInstance();
    static float accu = 0.f;
    accu += dt.asSeconds();

    world.update(dt);
    chat_text.update();
    chat_box.update(dt, chat_text.isFocused());
    view.setCenter(world.getPlayer()->getPosition());

    float t = 1.f - std::exp(-lerp_speed * dt.asSeconds());
    zoom += (target_zoom - zoom) * t;

    // Apply zoom to view
    view.setSize(original * zoom);

    while (accu >= FIXED_DT) {
        world.update_tick(dt);
        accu -= FIXED_DT;
    }
}

void GameState::update_event(const std::optional<sf::Event>& e) {
    if (!e.has_value()) return;  // optional is empty
    static bool ignore_next = false;
    auto* g = Game::getInstance();

    if (const auto* scroll = e->getIf<sf::Event::MouseWheelScrolled>()) {
        if (scroll->wheel == sf::Mouse::Wheel::Vertical) {
            target_zoom *= (scroll->delta > 0 ? 0.9f : 1.1f);
            target_zoom = std::clamp(target_zoom, 0.1f, 0.7f);
        }
    }
    if (const auto* key = e->getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::F && !chat_text.isFocused()) {
            world.addLight(
                world.worldToTileCoords(world.getPlayer()->getPosition()),
                10.f,
                sf::Color::White
            );
        }

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