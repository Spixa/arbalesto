#include "world.h"

#include "../entity/players.h"
#include "../entity/arrow.h"
#include "../game.h"

#include <algorithm>

World::World(std::string const& name) : name(name) {
    entities.reserve(1000); // TODO FIX: this is a temporary solution I need to fully move to Id's instead of ptrs and weak ptrs

    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            sf::Vector2i pos{x, y};

            if (x >= -1 && x <= 1 && y >= -1 && y <= 1) {
                // Center chunk = empty
                chunk.push_back(new Chunk({}, pos));
            } else {
                // Surrounding chunks = full water
                std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> data{};
                data.fill(Tile::Water);
                chunk.push_back(new Chunk(data, pos));
            }
        }
    }
}

World::~World() {

}

void World::addEntity(std::unique_ptr<Entity> entity) {
    entities.push_back(std::move(entity));
}

Entity* World::getPlayer() {
    return player;
}

bool World::isSolidAt(sf::Vector2f pos, sf::Vector2f size) const {
    sf::Vector2f half = size * 0.5f;

    // Check points inside the bounding box
    for (float x = pos.x - half.x; x < pos.x + half.x; x += TILE_SIZE/2.f) {
        for (float y = pos.y - half.y; y < pos.y + half.y; y += TILE_SIZE/2.f) {
            for (auto& c : chunk) {
                sf::Vector2f local = sf::Vector2f{x, y} - c->getOffset();
                int col = static_cast<int>(local.x / TILE_SIZE);
                int row = static_cast<int>(local.y / TILE_SIZE);

                if (c->isSolidTile(row, col))
                    return true;
            }
        }
    }

    return false;
}

Player* World::getNearestEntity(Entity* from) {
    if (!from) return nullptr;

    Player* nearest = nullptr;
    float minDistanceSq = std::numeric_limits<float>::max(); // use squared distance to avoid sqrt

    for (auto& e : entities) {
        if (!e || !e->isAlive() || e.get() == from)
            continue;

        if (auto* ai = dynamic_cast<Player*>(e.get())) {
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
    if (!player || !player->isAlive()) {
        if (auto* p = dynamic_cast<Entity*>(entities[0].get())) {
            player = p; // store non-owning pointer
        }
    }

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

    if (!ctrl_dead) {
        Game::getInstance()->setInfo("Enemies: " + std::to_string((nmes - 1)));

        if (nmes == 1) {
            Game::getInstance()->setInfo("You are the winner, my friend");
        }
    } else {
        Game::getInstance()->setInfo("You died\nYou are now spectating an AI with " + std::to_string((nmes - 1)) + " enemies");

        if (nmes == 0) {
            Game::getInstance()->setInfo("Very rare instance where everyone dies: Nobody wins!");
        } else if (nmes == 1) {
            Game::getInstance()->setInfo("this AI is the fittest and/or the luckiest. It has claimed victory");
        }
    }
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

        EntityId shooter = arrow->getShooterId();

        const sf::FloatRect arrowBounds = arrow->getBounds(); // GLOBAL bounds (AABB)

        // pass 2: test arrow against every entity
        nmes = 0;
        for (const auto& uptrE : entities) {
            Entity* e = uptrE.get();
            if (!e || !e->isAlive())
                continue;

            if (dynamic_cast<Arrow*>(uptrE.get())) // don't kill other arrows
                continue;
            nmes += 1;
            if (e->getId() == shooter) // don't kill the caster
                continue;


            if (overlaps(arrowBounds, e->getBounds())) {
                e->damage(10.f);

                if (e == entities[0].get() && !e->isAlive() && !ctrl_dead) {
                    ctrl_dead = true;
                }

                arrow->die();
                break;
            }
        }
    }
}
