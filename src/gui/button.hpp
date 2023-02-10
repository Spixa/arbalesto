#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

class Button : public sf::Drawable, public sf::Transformable{
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    std::function<void()> onClickEventFunction;
    sf::Vector2f size{};
    sf::Vector2f position{};

    // shapes
    sf::RectangleShape buttonRect;
    sf::Text buttonText;
    sf::Font buttonTextFont;

    // colors
    sf::Color defaultColor{};
    sf::Color hoverColor{};
    sf::Color clickColor{};
public:
    Button(std::string const& text, std::function<void()> clickEvent)
    {

    }

    void update() {
        
    }

    void setOnClickEvent(const std::function<void()>& f);
};