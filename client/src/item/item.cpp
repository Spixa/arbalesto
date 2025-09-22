#include "item.h"
#include "../game.h"

#include <cmath>
#include <SFML/Graphics.hpp>

sf::IntRect idx_to_rect(uint8_t index) {
    const int tile_size = 16;    // each tile is 16x16
    const int tiles_per_row = 4;  // atlas is 4 tiles per row (64 / 16)

    int x = (index % tiles_per_row) * tile_size;
    int y = (index / tiles_per_row) * tile_size;

    return sf::IntRect({x, y}, {tile_size, tile_size});
}

Item::Item(ItemType type) : type(type), item_display(*Game::getInstance()->getTextureManager().get("items")) {
    auto rect = idx_to_rect(static_cast<uint8_t>(type));
    item_display.setTextureRect(rect);
}

Item::~Item() {

}


void Item::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();

    target.draw(item_display, states);
}

void Item::update(sf::Time dt, bool facing) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
        onRMB();

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        onLMB();

    update_derived(dt, facing);

    if (facing) {
        auto s = item_display.getScale();
        item_display.setScale({std::fabs(s.x), std::fabs(s.y)});
    } else {
        auto s = item_display.getScale();
        item_display.setScale({std::fabs(s.x), -std::fabs(s.y)});
    }
}