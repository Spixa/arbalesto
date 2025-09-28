#pragma once

#include "item.h"
#include "../game.h"

class Entity;
class Flintlock : public Item {
public:
    Flintlock() : Item(ItemType::Flintlock), shoot(Game::getInstance()->getSoundManager().get("flintlock_shoot")), shooting_sound(*shoot) {
        cd_secs = 0.1f;
        item_display.setOrigin({8, 8});
        setPosition({0, 3});
        setScale({1, 1});
    }

    virtual ~Flintlock() {}

    void onLMB(sf::Vector2f const& dir, Entity* user) override;
    void update_derived(sf::Time dt, bool facing, sf::Vector2f const& dir) override;
private:
    std::shared_ptr<sf::SoundBuffer> shoot;
    sf::Sound shooting_sound;
};