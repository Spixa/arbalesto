#pragma once

#include "entity.h"
#include "../item/item.h"

class ItemStack : public Entity {
public:
    ItemStack(ItemType type, sf::Vector2f const& spawn);
    virtual ~ItemStack() = default;

    void update(sf::Time elapsed) override;
    void update_tick(sf::Time elapsed) override;

    sf::FloatRect getBounds() {
        if (!item) return sf::FloatRect();

        sf::Transform combined = getTransform() * item->getTransform();
        return combined.transformRect(item->getLocalBounds());
    }

    ItemType getType() const { return item->getType(); }
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
private:
    std::unique_ptr<Item> item;
    sf::Vector2f base_transform;
    float timer = 0.0;
};