#pragma once

#include <SFML/Graphics.hpp>

enum class ItemType: uint16_t {
    WoodSword = 0,
    CopperSword = 1,
    IronSword = 2,
    GoldSword = 3,
    DiamondSword = 4,
    EmeraldSword = 5,
    AmethystSword = 6,
    Bow = 7,
    Flintlock = 8,
    Arrow = 9,
    HealthPotion = 14,
};

class Entity;
class Item : public sf::Drawable, public sf::Transformable {
public:
    Item(ItemType type);
    virtual ~Item();

    ItemType getType() { return type;}
    void update(sf::Time dt, bool facing, sf::Vector2f const& dir, Entity* user);
    sf::FloatRect getLocalBounds() const { return item_display.getLocalBounds(); }
protected:
    virtual void update_derived(sf::Time dt, bool facing, sf::Vector2f const& dir) {};
    virtual void onRMB(sf::Vector2f const& dir, Entity* user) {};
    virtual void onLMB(sf::Vector2f const& dir, Entity* user) {};
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    ItemType type;
    sf::Clock cooldown;
    float cd_secs;
    sf::Sprite item_display;
};