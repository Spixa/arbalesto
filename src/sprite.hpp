#pragma once
#include <SFML/Graphics.hpp>
class HitboxSprite : public sf::Drawable, public sf::Transformable {
public:
    /// sets the hitbox
    void setHitbox(const sf::FloatRect& hitbox) {
        m_hitbox = hitbox;
    }
    /// gets the hitbox (use this instead of getGlobalBounds())
    sf::FloatRect getGlobalHitbox() const {
        return getTransform().transformRect(m_hitbox);
    }

    virtual void update(sf::Time deltaTime) = 0;
protected: 
    virtual void draw(sf::RenderTarget&, sf::RenderStates) const override = 0;
private:
    sf::FloatRect m_hitbox;
};