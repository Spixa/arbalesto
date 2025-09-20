#pragma once

#include <SFML/Graphics.hpp>

struct TileLightSource {
    sf::Vector2i position;
    float radius;
    sf::Color color;
};

struct LightTile {
    float intensity = 0.f;
};

namespace arb {
    struct Vector2iHash {
        std::size_t operator()(sf::Vector2i const& v) const noexcept {
            std::size_t h1 = std::hash<int>()(v.x);
            std::size_t h2 = std::hash<int>()(v.y);
            return h1 ^ (h2 << 1);
        }
    };
}