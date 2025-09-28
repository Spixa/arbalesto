#pragma once
#include "item.h"

#include <cmath>

class Entity;
class Sword : public Item {
public:
    Sword() : Item(ItemType::GoldSword) {
        item_display.setOrigin({8, 8});

        setPosition({0, 4});
        setScale({0.6, 0.6});
    }

    virtual ~Sword() {}

    void update_derived(sf::Time dt, bool facing, sf::Vector2f const& dir) override {
        if (!swinging) return;

        if (facing) {
            angle += swing_speed * dt.asSeconds();
            if (angle >= sf::degrees(90)) {
                swinging = false;
            }
        } else {
            angle -= swing_speed * dt.asSeconds();
            if (angle <= sf::degrees(-270)) {
                swinging = false;
            }
        }

        float r = angle.asRadians();
        sf::Vector2f offset(std::cos(r) * rad, std::sin(r) * rad);
        item_display.setPosition(offset);
        item_display.setRotation(angle);
    }
protected:
    void onLMB(sf::Vector2f const& dir, Entity* user) override {
        if (!swinging) {
            swinging = true;
            angle = sf::degrees(-90);
        }
    }
private:
    float rad = 5.0f;
    sf::Angle angle = sf::degrees(0);
    sf::Angle swing_speed = sf::degrees(1080);


    bool swinging = false;
};