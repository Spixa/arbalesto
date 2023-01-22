#pragma once
#include "state.hpp"

class StateManager {
public:
    StateManager() {
        State::Ptr menu_scene = std::make_shared<>();
    }
private:
    std::vector<State::Ptr> states_;
};