#include "entity.h"
#include "../game.h"

Entity::Entity(EntityId id, EntityType type, float health) : health(health), initial_health(health), id(id), type(type), facing(Facing::South) {
    alive = true;
}

Entity::~Entity() {
    // std::cout << "Gooddbye! I was " << getId() << std::endl;
}

Facing Entity::getFacing() {
    int sx = (velocity.x > 0) - (velocity.x < 0); // signum of x
    int sy = (velocity.y > 0) - (velocity.y < 0); // signum of y

    if (sx == 0 && sy == 0) return Facing::None;

    if (sx == 0 && sy < 0) return Facing::North;
    if (sx == 0 && sy > 0) return Facing::South;
    if (sx > 0 && sy == 0) return Facing::East;
    if (sx < 0 && sy == 0) return Facing::West;

    if (sx > 0 && sy < 0) return Facing::Northeast;
    if (sx < 0 && sy < 0) return Facing::Northwest;
    if (sx > 0 && sy > 0) return Facing::Southeast;
    if (sx < 0 && sy > 0) return Facing::Southwest;

    return Facing::None; // fallback
}
