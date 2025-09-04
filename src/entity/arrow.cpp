#include "arrow.h"
#include "../game.h"

#include <cmath>

Arrow::Arrow(sf::Vector2f spawn, sf::Vector2f dir, float speed, sf::Time lifetime, EntityId owner)
    : Entity(next(), EntityType::ArrowEntity, 1.f), lifetime(lifetime), sprite(*Game::getInstance()->getTextureManager().get("arrow")), shooter(owner) {
    setPosition(spawn);
    shooter = owner;
    if (dir.length() != 0.f) dir /= dir.length();
    velocity = dir * speed;

    sprite.setOrigin({8, 8});
    angle = sf::radians(std::atan2(dir.y, dir.x));
    angle += sf::degrees(45);
    sprite.setRotation(angle);
}

void Arrow::update(sf::Time dt) {
    move(velocity * dt.asSeconds());
    age += dt;

    if (age >= lifetime) die();
}

void Arrow::update_tick(sf::Time elapsed) {}

void Arrow::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(sprite, states);
}

