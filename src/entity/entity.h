#pragma once

#include <SFML/Graphics.hpp>

enum class EntityType {
    PlayerEntity,
    SpiderEntity,
    ArrowEntity,
};

enum class Facing {
    North,
    South,
    East,
    West,
    Northeast,
    Northwest,
    Southeast,
    Southwest,
    None
};

using EntityId = std::uint32_t;
class Entity : public sf::Drawable, public sf::Transformable {
public:
    Entity(EntityId id, EntityType type, float health);
    virtual ~Entity();

    virtual void update(sf::Time elapsed) = 0;
    virtual void update_tick(sf::Time elapsed) = 0;

    virtual sf::FloatRect getBounds() = 0;
    EntityId getId() const { return id; }

    void damage(float amount) {
        if (!invincible) {
            health -= amount;
            if (health <= 0.f) {
                die();
            }
        }
    }
    void die() { alive = false; }
    bool isAlive() { return alive; }
    void setInvincible(bool tof) { invincible = tof; }
    bool isInvincible() const { return invincible; }
    Facing getFacing();
protected:
    virtual void draw(sf::RenderTarget&, sf::RenderStates) const override = 0;
protected:
    EntityType type;
    Facing facing;
    sf::Vector2f velocity;
    bool alive{true};
    float health{40.f};
    float initial_health{40.f};
    bool invincible{false};

    EntityId id;
    static EntityId next() {
        static EntityId currentId = 0;
        return ++currentId;
    }
};