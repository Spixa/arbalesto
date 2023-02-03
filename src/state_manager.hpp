#pragma once
#include "state.hpp"

#include <memory>
#include "game_state.hpp"
class StateManager {
public:
    StateManager() {
        State::Ptr game = std::make_shared<GameState>();
        states_.push_back(std::move(game));

        selected_ = 0;
    }

    void update(sf::Time deltaTime) {
        if (states_[selected_]) {
            states_[selected_]->update(deltaTime);
        }
    }

    void render(sf::RenderWindow& window) {
        if (states_[selected_])
            window.draw(*states_[selected_].get());
    }
private:
    std::vector<State::Ptr> states_;
    size_t selected_;
};