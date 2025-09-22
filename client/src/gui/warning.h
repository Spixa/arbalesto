#pragma once

#include <SFML/Graphics.hpp>

class Warning : public sf::Drawable, public sf::Transformable {
public:
    Warning(sf::Font& font, float dura = 5.f);
    virtual ~Warning() = default;

    void sendWarn(sf::String const& msg);
    // void sendError(sf::String const& msg);
    void update(sf::Time dt);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
private:
    sf::Text text;
    sf::RectangleShape bg;

    sf::Clock timer;
    float dura;
    bool active = false;

    static inline float cubic_ease(float t) {
        t = std::clamp(t, 0.f, 1.f);
        t = 1.f - t*t*t;
        return t;
    }

    void update_bg();
};