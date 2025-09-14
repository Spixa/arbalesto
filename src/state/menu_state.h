#pragma once

#include "state.h"

#include "../gui/textedit.h"
#include "../gui/button.h"

class MenuState : public State {
public:
    MenuState();
    virtual ~MenuState() = default;
public:
    void start() override;
    void update(sf::Time) override {}
    void update_event(std::optional<sf::Event> const& e) override;
    void render_gui(sf::RenderTarget&) override;

    void render(sf::RenderTarget&) override;
private:
    Button play;
    sf::RectangleShape test{};
};