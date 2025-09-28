#pragma once

#include "item.h"
#include "../game.h"

class Bow : public Item {
public:
    Bow() : Item(ItemType::Bow) {
        item_display.setOrigin({8, 8});
        cd_secs = 0.125f;
    }

    virtual ~Bow() {}

    void onLMB(sf::Vector2f const& dir, Entity* user) override;

    void update_derived(sf::Time dt, bool facing, sf::Vector2f const& dir) override;
};