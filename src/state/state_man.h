#pragma once

#include <SFML/Graphics.hpp>
#include "state.h"
#include "game_state.h"

class StateManager {
public:
    StateManager();

    void init();
    void update(sf::Time elapsed);
    void render(sf::RenderWindow& window);

    sf::View& getCurrentView() {
        return states[selected]->view;
    }


    GameState* getGameState() {
        return dynamic_cast<GameState*>(states[0].get());
    }

private:
    std::vector<State::Ptr> states;
    size_t selected{0};
};