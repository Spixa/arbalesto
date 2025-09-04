#pragma once

#include <SFML/Graphics.hpp>

enum class ItemType: uint8_t {
    WoodSword = 0,
    CopperSword = 1,
    IronSword = 2,
    GoldSword = 3,
    DiamondSword = 4,
    EmeraldSword = 5,
    AmethystSword = 6,
    Bow = 7,
    Bow2 = 8,
    Arrow = 9,
};

class Item : public sf::Drawable, public sf::Transformable {
public:
    Item(ItemType type);
    virtual ~Item();

    ItemType getType() { return type;}
    void update(sf::Time dt, bool facing);
protected:
    virtual void update_derived(sf::Time dt, bool facing) = 0;
    virtual void onRMB() {};
    virtual void onLMB() {};
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    ItemType type;
    sf::Sprite item_display;
};