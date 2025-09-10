#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

class TextEdit : public sf::Drawable, public sf::Transformable {
public:
    using SubmitCallback = std::function<void(sf::String const&)>;

    TextEdit(sf::String const& prefix, sf::Font const& font, unsigned int char_size = 14.f);
    virtual ~TextEdit() = default;

    void input(char c);
    void update();

    sf::String consumeSubmitted() {
        auto tmp = last_submitted;
        last_submitted.clear();

        return tmp;
    }

    sf::FloatRect getLocalBounds() const { return text.getLocalBounds(); }
    sf::FloatRect getGlobalBounds() const { return getTransform().transformRect(text.getGlobalBounds()); }
    void setSubmitCallback(SubmitCallback cb) { this->cb = cb; }
    void setSize(const sf::Vector2f& size) { background.setSize(size); }
    void setFocused(bool tof) { focused = tof; }
    bool isFocused() const { return focused; }
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
private:
    sf::Text text;
    sf::RectangleShape background;
private:
    SubmitCallback cb;
private:
    sf::String prefix;
    sf::String content;
    sf::String last_submitted;

    sf::Clock blink{};
    bool cursor = false;
    bool focused = false;

    void update_text();
};