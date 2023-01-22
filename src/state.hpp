#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>

enum class States {
    GameStateType,
    MenuStateType,
    SettingsStateType,
    AboutStateType
};

class State : public sf::Drawable, public sf::Transformable {
public:
    State(std::string const& name, States type) : name_(name), type_(type) {}
    virtual ~State() {}
    using Ptr = std::shared_ptr<State>;
public:
    States getState() const { return type_; }
    std::string getName() const { return name_; }
public:
    virtual void update(sf::Time deltaTime) = 0;
protected:
    virtual void draw(sf::RenderTarget&, sf::RenderStates) const override = 0;
protected:
    std::vector<sf::Drawable> drawables;
private:
    std::string name_;
    States type_;
};

