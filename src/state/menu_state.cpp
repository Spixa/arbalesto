#include "menu_state.h"

#include "../game.h"

MenuState::MenuState() : State("menu"), play("Play", Game::getInstance()->getFallbackFont(), {100, 300}) {

}

void MenuState::start() {
    info("Started menu state");
}

void MenuState::update_event(std::optional<sf::Event> const& e) {
    if (!e) return;
    play.handleEvent(*e);

    if (play.getState() == Button::State::Clicked) {
        Game::getInstance()->getStateManager().setState(StateId::GAMESTATE);
    }
}

void MenuState::render_gui(sf::RenderTarget& target) {
    target.draw(play);
}

void MenuState::render(sf::RenderTarget& target) {

}