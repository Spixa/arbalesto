#pragma once

#include "item.h"
#include "../game.h"

class Bow : public Item {
public:
    Bow() : Item(ItemType::Bow) {
        item_display.setOrigin({8, 8});
        // setPosition({8, 12});

    }

    virtual ~Bow() {}

    void onLMB() override {

    }

    void update_derived(sf::Time dt, bool facing) override {

    }
};