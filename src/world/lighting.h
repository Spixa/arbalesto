#pragma once

#include <SFML/Graphics.hpp>

struct LightSource {
    sf::Vector2f position;
    float radius;
    sf::Color color;
};

struct LightTile {
    float intensity = 0.f;
};