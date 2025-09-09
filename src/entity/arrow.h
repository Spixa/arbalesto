#pragma once

#include "entity.h"

class Arrow : public Entity {
public:
    Arrow(sf::Vector2f spawn, sf::Vector2f dir, float speed, sf::Time lifetime, EntityId owner, sf::Vector2f initial = {0.0, 0.0});
    virtual ~Arrow() = default;

    void update(sf::Time elapsed) override;
    void update_tick(sf::Time elapsed) override;

    void onCollision();
    EntityId getShooterId() const { return shooter; }
    sf::FloatRect getBounds() {
        sf::Transform combined = getTransform() * sprite.getTransform();
        return combined.transformRect(sprite.getLocalBounds());
    }
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
    sf::Sprite sprite;
    sf::Time lifetime;
    sf::Time age{sf::Time::Zero};
    sf::Angle angle;
private:
    EntityId shooter{};
};