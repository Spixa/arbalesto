#pragma once

#include "item.h"
#include "../game.h"

class Flintlock : public Item {
public:
    Flintlock() : Item(ItemType::Flintlock) {
        item_display.setOrigin({8, 8});
        setPosition({0, 3});
        setScale({1, 1});
    }

    virtual ~Flintlock() {}

    void onLMB() override {

    }

    void update_derived(sf::Time dt, bool facing) override {

    }
};