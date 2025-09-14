#pragma once

#include <SFML/Graphics.hpp>

// implementation taken out of github.com/Spixa/Game's RichButton
class Button : public sf::Drawable, public sf::Transformable {
public:
    enum class Style {
        Default
    };

    enum class State {
        Normal,
        Hovered,
        Clicked
    };

    Button(sf::String const& content, sf::Font& font, sf::Vector2f const& position, Style style = Style::Default);
    virtual ~Button() = default;
public:
    void setContent(sf::String const& c) { content = c; }
    void setHoverContent(sf::String const& hc) { hover_content = hc; }
    void setStyle(Style s) { style = s; }

    State getState() const { return state; }
public:
    void handleEvent(sf::Event const& e);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
private:
    sf::String content, hover_content;
    Style style{Style::Default};
    State state{State::Normal};
private:
    sf::Color bg_normal, bg_hover, bg_clicked;
    sf::Color txt_normal, txt_hover, txt_clicked;

    sf::RectangleShape button;
    sf::Text text, shadow;
};