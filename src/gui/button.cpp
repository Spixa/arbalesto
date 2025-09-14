#include "button.h"

#include "../game.h"

Button::Button(sf::String const& content, sf::Font& font, sf::Vector2f const& position, Style style)
    : text(font), shadow(font), style(style), state(State::Normal)
{
    setPosition(position);

    switch (style) {
        case Style::Default: default: {
            // fallback style
            txt_normal = sf::Color(255,255,255);
            txt_hover = sf::Color(255,255,255);
            txt_clicked = sf::Color(255,255,255);
            bg_normal = sf::Color(255,255,255,100);
            bg_hover = sf::Color(200,200,200,100);
            bg_clicked = sf::Color(150,150,150);
        } break;
    }

    text.setString(content);
    shadow.setString(content);
    text.setFillColor(txt_normal);
    shadow.setFillColor(sf::Color(0,0,0,150));

    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({0, bounds.size.y / 2.f});
    shadow.setOrigin({0, bounds.size.y / 2.f});

    sf::Vector2f size = bounds.size * 2.f;
    button.setSize(size);
    button.setOrigin(bounds.size / 2.f);
    button.setFillColor(bg_normal);

    // offset
    shadow.setPosition({5.f, 2.f});
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(button, states);
    target.draw(shadow, states);
    target.draw(text, states);
}

void Button::handleEvent(sf::Event const& e) {
    auto mouse = Game::getInstance()->getWindow().mapPixelToCoords(sf::Mouse::getPosition(Game::getInstance()->getWindow()));
    // Hitbox check
    sf::Transform combined = getTransform() * button.getTransform();
    bool hovered = combined.transformRect(button.getLocalBounds()).contains(mouse);

    bool mouse_released = e.is<sf::Event::MouseButtonReleased>();
    if (hovered) {
        if (mouse_released) {
            state = State::Clicked;
            button.setFillColor(bg_clicked);
            text.setFillColor(txt_clicked);
        } else {
            state = State::Hovered;
            button.setFillColor(bg_hover);
            text.setFillColor(txt_hover);
        }
    } else {
        state = State::Normal;
        button.setFillColor(bg_normal);
        text.setFillColor(txt_normal);
    }
}
