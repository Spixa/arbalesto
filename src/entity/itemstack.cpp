#include "itemstack.h"

#include <cmath>

ItemStack::ItemStack(ItemType type, sf::Vector2f const& spawn) : Entity(next(), EntityType::ItemEntity, 2000.0), base_transform(spawn), item(std::make_unique<Item>(type)) {
    setPosition(spawn);
}

void ItemStack::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();

    target.draw(*item, states);
}

void ItemStack::update(sf::Time dt) {
    timer += dt.asSeconds();

    constexpr float amplitude = 2.5f; // in px
    constexpr float speed = 3.f; // omega (2pi * freq)

    float offset = std::sin(timer * speed) * amplitude;

    setPosition(base_transform + sf::Vector2f(0.f, offset)); // vertical
}

void ItemStack::update_tick(sf::Time dt) {

}