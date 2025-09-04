#pragma once

#include <SFML/Graphics.hpp>

class State {
public:
    State(std::string const& name) : name(name) {}
    virtual ~State() {}

    using Ptr = std::shared_ptr<State>;
    sf::View view{};
public:
    virtual void start() = 0;
    virtual void update(sf::Time elapsed) = 0;
    virtual void render(sf::RenderTarget&) = 0;
protected:
    std::vector<sf::Drawable> drawables;
private:
    std::string name;
};