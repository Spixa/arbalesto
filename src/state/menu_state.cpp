#include "menu_state.h"

#include "../game.h"

MenuState::MenuState() : State("menu"), play("Play", Game::getInstance()->getFallbackFont(), {100, 300}), username("Username: ", Game::getInstance()->getFallbackFont(), 30) {
    username.setSubmitCallback([this](const sf::String& msg) {
        if (!msg.isEmpty()) {
            sel_uname = msg;
            Game::getInstance()->sendWarning("Username set to " + msg);
        } else {
            Game::getInstance()->sendWarning("Please don't set an empty username");
        }
    });
    username.setFocused(true);

    sf::FloatRect ui = Game::getInstance()->getUIBounds();
    username.setSize({ui.size.x, 30.f});

    float padding = 20.f;
    username.setPosition({
        ui.position.x + 5.f,
        ui.position.y + ui.size.y - username.getLocalBounds().size.y - padding
    });
}

void MenuState::start() {
    info("Started menu state");
}

void MenuState::update(sf::Time dt) {
    username.update();
}

void MenuState::update_event(std::optional<sf::Event> const& e) {
    if (!e) return;
    play.handleEvent(*e);

    if (play.getState() == Button::State::Clicked) {
        if (!sel_uname.isEmpty()) {
            Game::getInstance()->initNetworking(sel_uname);
            Game::getInstance()->getStateManager().setState(StateId::GAMESTATE);

        } else {
            Game::getInstance()->sendWarning("Please select a username");
            username.setFocused(true);
        }
    }

    if (const auto* text = e->getIf<sf::Event::TextEntered>()) {
        if (username.isFocused() && text->unicode < 128) {
            username.input(static_cast<char>(text->unicode));
        }
    }
}

void MenuState::render_gui(sf::RenderTarget& target) {
    target.draw(play);
    target.draw(username);
}

void MenuState::render(sf::RenderTarget& target) {

}