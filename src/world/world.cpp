#include "world.h"

#include "../entity/players.h"
#include "../entity/arrow.h"
#include "../item/sword.h"
#include "../entity/itemstack.h"
#include "../game.h"

#include <algorithm>
#include <random>
#include <cmath>

World::World(std::string const& name) : name(name) {
    entities.reserve(1000); // TODO FIX: this is a temporary solution I need to fully move to Id's instead of ptrs and weak ptrs

    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            sf::Vector2i pos{x, y};
            std::unique_ptr<Chunk> c;

            // Try loading chunk from disk
            Chunk temp({}, pos);
            if (temp.load()) {
                c = std::make_unique<Chunk>(temp);
            } else {
                // If not found, generate new
                if (x >= -1 && x <= 1 && y >= -1 && y <= 1) {
                    c = std::make_unique<Chunk>(std::array<Tile, CHUNK_SIZE*CHUNK_SIZE>{}, pos);
                } else {
                    std::array<Tile, CHUNK_SIZE*CHUNK_SIZE> data{};
                    data.fill(Tile::Water);
                    c = std::make_unique<Chunk>(data, pos);
                }

                c->save();
            }

            chunk.push_back(c.release());
        }
    }

    spawn_loot();
}

World::~World() {

}

void World::addEntity(std::unique_ptr<Entity> entity) {
    if (dynamic_cast<ControllingPlayer*>(entity.get()))
        player = entity.get();

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
    float min_dist = 1000.f;
    float minDistanceSq = min_dist * min_dist; // use squared distance to avoid sqrt

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

    check_collisions();
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](auto& e){ return !e->isAlive(); }),
        entities.end()
    );

    if (!ctrl_dead) {
        Game::getInstance()->setInfo("Enemies: " + std::to_string((nmes - 1)));

        if (nmes == 1) {
            Game::getInstance()->setInfo("You won");
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

    static Tile pref;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) {
        pref = Tile::Grass;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) {
        pref = Tile::Water;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) {
        pref = Tile::Cobble;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        sf::Vector2f mouse = Game::getInstance()->getMouseWorld();
        for (auto&c : chunk) {
            c->update_tick(mouse, pref);
        }
    }

    if (chunk_idle.getElapsedTime() >= sf::seconds(2.f)) {
        save_dirty_chunks();
        chunk_idle.restart();
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

void World::spawn_loot() {
    std::vector<sf::Vector2f> candid;

    for (auto& c : chunk) {
        for (int row = 0; row < CHUNK_SIZE; ++row) {
            for (int col = 0; col < CHUNK_SIZE; ++col) {
                Tile t = c->getTile(row, col);
                if (t != Tile::Water) {
                    // tile center in world space
                    sf::Vector2f pos = c->getOffset() +
                        sf::Vector2f{col * TILE_SIZE + TILE_SIZE/2.f,
                                    row * TILE_SIZE + TILE_SIZE/2.f};
                    candid.push_back(pos);
                }
            }
        }
    }


    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(candid.begin(), candid.end(), rng);
    int num = std::min(40, (int) candid.size());

    for (int i = 0; i < num; ++i) {
        sf::Vector2f spawn = candid[i];

        addEntity(std::make_unique<ItemStack>(ItemType::HealthPotion, spawn));
    }
}


void World::check_collisions() {
    std::vector<Arrow*> arrows;
    std::vector<ItemStack*> items;
    std::vector<Player*> players;

    // classify entities once
    for (auto& uptr : entities) {
        if (!uptr || !uptr->isAlive())
            continue;

        if (auto* a = dynamic_cast<Arrow*>(uptr.get())) arrows.push_back(a);
        else if (auto* i = dynamic_cast<ItemStack*>(uptr.get())) items.push_back(i);
        else if (auto* p = dynamic_cast<Player*>(uptr.get())) players.push_back(p);
    }

    // --- arrows vs players ---
    for (auto* arrow : arrows) {
        EntityId shooter = arrow->getShooterId();
        const sf::FloatRect arrowBounds = arrow->getBounds();

        for (auto* p : players) {
            if (p->getId() == shooter) continue; // don’t hit yourself
            if (overlaps(arrowBounds, p->getBounds())) {
                p->damage(10.f);
                if (p == entities[0].get() && !p->isAlive() && !ctrl_dead) {
                    ctrl_dead = true;
                }
                arrow->die();
                break;
            }
        }
    }

    // --- items vs players ---
    for (auto* item : items) {
        for (auto* p : players) {
            if (overlaps(item->getBounds(), p->getBounds())) {
                p->pickup(item->getType());
                item->die();
                break;
            }
        }
    }

    nmes = players.size();
}

void World::save_dirty_chunks() {
    for (auto& c : chunk) {
        if (c->isDirty()) {
            c->save();
            c->tidy();
        }
    }
}