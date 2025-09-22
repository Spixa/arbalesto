#pragma once

#include <SFML/Graphics.hpp>
#include "state.h"
#include "game_state.h"

enum class StateId : size_t {
    MENUSTATE = 0,
    GAMESTATE = 1,
};

class StateManager {
public:
    StateManager();

    void init();
    void update(sf::Time elapsed);
    void update_event(std::optional<sf::Event> const& e);
    void render(sf::RenderWindow& window);
    void render_gui(sf::RenderWindow& window);

    sf::View& getCurrentView() {
        return states[selected]->view;
    }


    GameState* getGameState() {
        return dynamic_cast<GameState*>(states[static_cast<size_t>(StateId::GAMESTATE)].get());
    }

    bool setState(StateId id) {
        size_t tmp = selected;
        selected = static_cast<size_t>(id);

        if (states[selected]) {
            states[selected]->start();
            return true;
        } else {
            selected = tmp;
            return false;
        }
    }
private:
    std::vector<State::Ptr> states;
    size_t selected{0};
};