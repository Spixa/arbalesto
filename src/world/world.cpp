#include "world.h"

#include "../entity/players.h"
#include "../entity/arrow.h"
#include "../game.h"

#include <algorithm>

World::World(std::string const& name) : name(name) {
    entities.reserve(1000); // TODO FIX: this is a temporary solution I need to fully move to Id's instead of ptrs and weak ptrs
    chunk.push_back(new Chunk({}, {0, -1}));
    chunk.push_back(new Chunk({}, {0, 0}));
    chunk.push_back(new Chunk({}, {-1, -1}));
    chunk.push_back(new Chunk({}, {-1, 0}));
}

World::~World() {

}

void World::addEntity(std::unique_ptr<Entity> entity) {
    if (!player) {
        if (auto* p = dynamic_cast<ControllingPlayer*>(entity.get())) {
            player = p; // store non-owning pointer
        }
    }
    entities.push_back(std::move(entity));
}

ControllingPlayer* World::getPlayer() {
    return player;
}

AiPlayer* World::getNearestAiPlayer(Entity* from) {
    if (!from) return nullptr;

    AiPlayer* nearest = nullptr;
    float minDistanceSq = std::numeric_limits<float>::max(); // use squared distance to avoid sqrt

    for (auto& e : entities) {
        if (!e || !e->isAlive() || e.get() == from)
            continue;

        if (auto* ai = dynamic_cast<AiPlayer*>(e.get())) {
            sf::Vector2f diff = ai->getPosition() - from->getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;

            if (distSq < minDistanceSq) {
                minDistanceSq = distSq;
                nearest = ai;
            }
        }
    }

    return nearest; // nullptr if no other AI exists
}

void World::update(sf::Time dt) {
    for (auto& x : entities) {
        if (x && x->isAlive())
            x->update(dt);
    }

    check_arrow_collisions();
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](auto& e){ return !e->isAlive(); }),
        entities.end()
    );
}

void World::update_tick(sf::Time elapsed) {
    for (auto& x : entities) {
        if (x && x->isAlive())
            x->update_tick(elapsed);
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        sf::Vector2f mouse = Game::getInstance()->getMouseWorld();
        for (auto&c : chunk) {
            c->update_tick(mouse);
        }
    }

}

void World::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();

    for (auto& c: chunk) {
        c->render(target);
    }

    for (auto& x : entities) {
        if (x)
            target.draw(*x, states);
    }
}


inline bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b) {
    const float aLeft   = a.position.x;
    const float aTop    = a.position.y;
    const float aRight  = aLeft + a.size.x;
    const float aBottom = aTop  + a.size.y;

    const float bLeft   = b.position.x;
    const float bTop    = b.position.y;
    const float bRight  = bLeft + b.size.x;
    const float bBottom = bTop  + b.size.y;

    // strict comparisons: touching edges do NOT count as a hit
    return (aLeft < bRight) && (aRight > bLeft) && (aTop < bBottom) && (aBottom > bTop);
    // if you want “touching counts”, switch < and > to <= and >=.
}


void World::check_arrow_collisions() {
    for (const auto& uptrA : entities) {
        auto* arrow = dynamic_cast<Arrow*>(uptrA.get());
        if (!arrow || !arrow->isAlive())
            continue;

        EntityId shooter = arrow->getShooterId(); // may be nullptr; consider id/weak_ptr in your design

        const sf::FloatRect arrowBounds = arrow->getBounds(); // GLOBAL bounds (AABB)

        // pass 2: test arrow against every entity
        for (const auto& uptrE : entities) {
            Entity* e = uptrE.get();
            if (!e || !e->isAlive())
                continue;

            if (dynamic_cast<Arrow*>(uptrE.get())) // don't kill other arrows
                continue;
            if (e->getId() == shooter) // don't kill the caster
                continue;

            // if (e == entities[0].get()) // FOR DEBUGGING
            //     continue;

            if (overlaps(arrowBounds, e->getBounds())) {
                e->damage(10.f);
                arrow->die();
                break;
            }
        }
    }
}
