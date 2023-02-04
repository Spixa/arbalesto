#include "menu_state.hpp"
#include "state_manager.hpp"

MenuState::MenuState() : State("menu", States::MenuStateType)
{

}

void MenuState::update(sf::Time deltaTime) {

}

void MenuState::draw(sf::RenderTarget& targ, sf::RenderStates states) const {
    states.transform *= getTransform();

}