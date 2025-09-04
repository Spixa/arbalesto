#pragma once

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable & operator=(const NonCopyable&) = delete;
};

// put in some utility header
// template <typename DrawableT>
// sf::FloatRect transformedGlobalBounds(const DrawableT& drawable, const sf::Transformable& parent) {
//     // local bounds of the drawable (before any transform)
//     sf::FloatRect local = drawable.getLocalBounds();
//     // combined transform: parent (e.g. Player) then drawable's own transform
//     sf::Transform combined = parent.getTransform() * drawable.getTransform();
//     return combined.transformRect(local);
// }
