#pragma once

#include <SFML/Graphics.hpp>

enum class StaticObjectType: uint16_t {
    Torch,
    WoodenDoor,
    Tree,
    Fence
};

struct StaticObject {
    sf::Vector2i origin, size;
    sf::IntRect atlas_rect;
    StaticObjectType type;
    bool solid = true;
    bool emit = false;
    float light_radius = 0.f;

    sf::FloatRect getWorldBounds(sf::Vector2f const& chunk_offset) const;
};