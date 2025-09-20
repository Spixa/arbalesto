#pragma once

#include "entity.h"
#include "../animation/sprite.h"
#include "../item/item.h"

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

    void pickup(ItemType type);
    sf::FloatRect getBounds() override { return sprite.getGlobalBounds(getTransform()); }
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
protected:
    std::unique_ptr<Item> holding;
    sf::RectangleShape health_box;
    sf::RectangleShape health_bar;
    AnimatedSprite sprite;
    int row;
    bool inv;
    bool arrow{false};
    sf::Clock arrow_cooldown;
};

class AiPlayer : public Player {
public:
    AiPlayer(sf::Vector2f spawn) : Player(ItemType::Bow, spawn) {

    }

    virtual ~AiPlayer() {}

    void update_tick_derived(sf::Time dt) override;
private:
    sf::Clock decisionClock;
    sf::Time decisionInterval;

    sf::Time arrowInterval;
    sf::Clock arrowCooldown;

    Entity* currentTarget{nullptr};
    sf::Vector2f holdingDirection = {1.f, 0.f};
};

class ControllingPlayer : public Player {
public:
    ControllingPlayer() : Player(ItemType::Bow, {0, 0}, 100) { setInvincible(true); /* TODO: for testing, remove this */ }
    virtual ~ControllingPlayer();
    void update_derived(sf::Time) override;
};