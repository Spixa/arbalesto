#include "state_man.h"
#include "game_state.h"

StateManager::StateManager() {

}

void StateManager::init() {
    State::Ptr game_state = std::make_shared<GameState>();
    states.push_back(game_state);

    for (auto x : states) {
        x->start();
    }
}

void StateManager::update(sf::Time dt) {
    if (states[selected]) {
        states[selected]->update(dt);
    }
}

void StateManager::update_event(std::optional<sf::Event> const& e) {
    if (states[selected]) {
        states[selected]->update_event(e);
    }
}

void StateManager::render(sf::RenderWindow& window) {
    if (states[selected]) {
        states[selected]->render(window);
    }
}