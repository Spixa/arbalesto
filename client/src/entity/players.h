#pragma once

#include "entity.h"
#include "../animation/sprite.h"
#include "../item/item.h"
#include "../particle/smokesystem.h"

#include "../net.h"
class Player : public Entity {
public:
    Player(ItemType holding, sf::Vector2f spawn, float health = 100.f);
    virtual ~Player();

    void update(sf::Time elapsed) override;
    void update_tick(sf::Time elapsed) override;

    virtual void update_derived(sf::Time elapsed) {}
    virtual void update_tick_derived(sf::Time elapsed) {}
    void setHolding(std::unique_ptr<Item> item) {
        holding = std::move(item);
    }
    bool isLeft() const { return inv; }
    void setDisplayname(sf::String const& dname) { displayname.setString(dname); }
    void pickup(ItemType type);
    sf::FloatRect getBounds() override { return sprite.getGlobalBounds(getTransform()); }

    void setVelocity(sf::Vector2f const& v) { velocity = v; }
    sf::Vector2f getVelocity() const { return velocity; }
    float getItemRotation() const {
        if (holding) return holding->getRotation().asRadians();
        return 0.f;
    }
    ItemType getHoldingType() const {
        if (holding) return holding->getType();
        return ItemType::Bow;
    }
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
protected:
    std::unique_ptr<Item> holding;
    sf::RectangleShape health_box;
    sf::RectangleShape health_bar;
    sf::Text displayname;
    AnimatedSprite sprite;
    int row;
    bool inv;
    bool arrow{false};
    sf::Clock arrow_cooldown;
};

class RemotePlayer : public Player {
public:
    RemotePlayer(PlayerState const& init);

    void update_derived(sf::Time dt) override;
    void setTarget(sf::Vector2f const& tpos, sf::Vector2f const& tvel, float trot, uint16_t titem);
    void shoot(sf::Vector2f const& dir);
private:
    PlayerState state;
    sf::Vector2f target;
    sf::Vector2f predicted_pos;
    sf::Vector2f last_predicted_pos, last_velocity;
    std::chrono::steady_clock::time_point last_seen;

    float target_rotation = 0.f;
    uint16_t target_item = 0;
};

// class AiPlayer : public Player {
// public:
//     AiPlayer(sf::Vector2f spawn) : Player(ItemType::Bow, spawn) {
//         pickup(ItemType::Bow);
//     }

//     virtual ~AiPlayer() {}

//     void update_tick_derived(sf::Time dt) override;
// private:
//     sf::Clock decisionClock;
//     sf::Time decisionInterval;

//     sf::Time arrowInterval;
//     sf::Clock arrowCooldown;

//     Entity* currentTarget{nullptr};
//     sf::Vector2f holdingDirection = {1.f, 0.f};
// };

class ControllingPlayer : public Player {
public:
    ControllingPlayer() : Player(ItemType::Bow, {0, 0}, 100) { setInvincible(true); /* TODO: for testing, remove this */ }
    virtual ~ControllingPlayer();
    void update_derived(sf::Time) override;
};