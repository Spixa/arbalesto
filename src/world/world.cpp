#include "world.h"

#include "../entity/players.h"
#include "../entity/arrow.h"
#include "../item/sword.h"
#include "../entity/itemstack.h"
#include "../game.h"

#include <algorithm>
#include <random>
#include <queue>
#include <cmath>

namespace {
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

    // Inefficient, don't call in loops
    sf::Texture generateRadialGradient(unsigned size = 256) {
        std::vector<uint8_t> pixels;
        pixels.resize(size * size * 4); // R G B A -> 4 bytes

        const float half = size / 2.f;
        const float inv_half = 1.f / half;

        for (unsigned y = 0; y < size; ++y) {
            for (unsigned x = 0; x < size; ++x) {
                float nx = (static_cast<float>(x) - half) * inv_half;
                float ny = (static_cast<float>(y) - half) * inv_half;
                float dist = std::sqrt(nx*nx + ny*ny);

                std::uint8_t alpha = 0;
                if (dist <= 1.f) {
                    float a = (1.f - dist); // falloff
                    // GPT's insight: apply an easing curve for smoother falloff: a = a * a * (3 - 2*a);
                    alpha = static_cast<std::uint8_t>(std::clamp(a, 0.f, 1.f) * 255.f);
                }

                std::size_t idx = (static_cast<std::size_t>(y) * size + x) * 4;
                pixels[idx + 0] = 255;   // R
                pixels[idx + 1] = 255;   // G
                pixels[idx + 2] = 255;   // B
                pixels[idx + 3] = alpha; // A
            }
        }
        sf::Texture tex{{size, size}};
        tex.update(pixels.data());
        tex.setSmooth(true);
        return tex;
    }
}


World::World(std::string const& name) : name(name) {
    entities.reserve(1000); // TODO FIX: this is a temporary solution I need to fully move to Id's instead of ptrs and weak ptrs
    light_tex = ::generateRadialGradient(256);
    lightmap = sf::RenderTexture(Game::getInstance()->getWindow().getSize());

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

void World::addLight(LightSource source) {
    lights.push_back(source);
}

Entity* World::getPlayer() {
    return player;
}

sf::Vector2i World::worldToTileCoords(const sf::Vector2f& pos) const {
    return sf::Vector2i(
        static_cast<int>(pos.x / TILE_SIZE),
        static_cast<int>(pos.y / TILE_SIZE)
    );
}

bool World::isValidTile(const sf::Vector2i& tile) const {
    for (auto& c : chunk) {
        int col = tile.x - static_cast<int>(c->getOffset().x / TILE_SIZE);
        int row = tile.y - static_cast<int>(c->getOffset().y / TILE_SIZE);
        if (row >= 0 && row < CHUNK_SIZE && col >= 0 && col < CHUNK_SIZE)
            return true;
    }
    return false;
}

void World::propagate_light(const LightSource& light, std::vector<std::vector<LightTile>>& grid) const {
    static const std::vector<sf::Vector2i> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1}
    };

    sf::Vector2i center = worldToTileCoords(light.position);
    int radius_tiles = static_cast<int>(std::ceil(light.radius / TILE_SIZE));

    std::queue<sf::Vector2i> queue;
    queue.push(center);
    grid[center.x][center.y].intensity = 1.f; // src tile is highest intensity

    while (!queue.empty()) {
        sf::Vector2i tile = queue.front(); queue.pop(); // take last tile
        float cur_i = grid[tile.x][tile.y].intensity;
        for (auto& dir : directions) {
            sf::Vector2i neighbor = tile + dir;
            if (!isValidTile(neighbor)) continue;
            if (grid[neighbor.x][neighbor.y].intensity > 0.f) continue;

            if (isSolidAt(sf::Vector2f(neighbor.x * TILE_SIZE, neighbor.y * TILE_SIZE), {TILE_SIZE, TILE_SIZE}))
                continue; // stop light

            float dist = std::sqrt(float((neighbor.x - center.x)*(neighbor.x - center.x) +
                                         (neighbor.y - center.y)*(neighbor.y - center.y)));
            float falloff = std::max(0.f, 1.f - dist / radius_tiles);
            if (falloff <= 0.f) continue;

            grid[neighbor.x][neighbor.y].intensity = falloff;
            queue.push(neighbor);
        }
    }
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
    if (grace.getElapsedTime().asSeconds() <= 10.f)
        return nullptr;
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

    // if (lights.size() > 0) lights[0].position = player->getPosition(); // follow player for test

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

sf::String World::getTimeOfDay() const {
    float t = static_cast<float>(time % DAY_LENGTH) / DAY_LENGTH;
    int total_mins = static_cast<int>(t * 24.f * 60.f + 0.5f); // round to nearest minute
    int hours = total_mins / 60; // pos
    int minutes = total_mins % 60; /// offset

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours
        << ":"
        << std::setw(2) << std::setfill('0') << minutes;

    return oss.str();
}

void World::update_tick(sf::Time elapsed) {
    time = (time + 1 ) % DAY_LENGTH;

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
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)) {
        pref = Tile::Wood;
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

    sf::View const& view = target.getView();
    constexpr float renderScale = 1.f;

    sf::Vector2f size = view.getSize() * renderScale;
    sf::Vector2f center = view.getCenter();

    sf::FloatRect viewRect(
        center - size / 2.f,
        size
    );

    for (auto& c: chunk) {
        if (::overlaps(viewRect, c->getBounds())) {
            c->render(target);
        }
    }

    for (auto& x : entities) {
        if (!x) continue;

        if (::overlaps(viewRect, x->getBounds())) {
            target.draw(*x, states);
        }
    }

    // light pass
    const sf::Vector2u window_size = target.getSize();
    lightmap.setView(target.getView());
    lightmap.clear(getAmbientLight());

    for (auto const& light : lights) {
        sf::Sprite glow{light_tex};
        glow.setOrigin({light_tex.getSize().x / 2.f, light_tex.getSize().y / 2.f});
        glow.setPosition(light.position);

        constexpr float f = 1.f;
        sf::Color boostedColor(
            std::min(255, int(light.color.r * f)),
            std::min(255, int(light.color.g * f)),
            std::min(255, int(light.color.b * f))
        );
        glow.setColor(boostedColor);

        float scale = (light.radius * 2.f) / static_cast<float>(light_tex.getSize().x);
        glow.setScale({scale, scale});

        lightmap.draw(glow, sf::BlendAdd);
    }

    lightmap.display(); // render the texture

    sf::Sprite lm_sprite{lightmap.getTexture()};
    lm_sprite.setPosition(target.mapPixelToCoords({0, 0}));

    lm_sprite.setScale({
        target.getView().getSize().x / window_size.x,
        target.getView().getSize().y / window_size.y
    });
    target.draw(lm_sprite, sf::BlendMultiply);
}

void World::spawn_loot() {
    std::vector<sf::Vector2f> candid;

    for (auto& c : chunk) {
        for (int row = 0; row < CHUNK_SIZE; ++row) {
            for (int col = 0; col < CHUNK_SIZE; ++col) {
                Tile t = c->getTile(row, col);
                if (t != Tile::Water && t != Tile::Cobble) {
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
            if (::overlaps(arrowBounds, p->getBounds())) {
                p->damage(10.f);
                if (dynamic_cast<ControllingPlayer*>(p) && !p->isAlive() && !ctrl_dead) {
                    ctrl_dead = true;
                    Game::getInstance()->tell("&6Player &fwas slain by an arrow");
                }

                arrow->die();
                break;
            }
        }
    }

    // --- items vs players ---
    for (auto* item : items) {
        for (auto* p : players) {
            if (::overlaps(item->getBounds(), p->getBounds())) {
                p->pickup(item->getType());
                item->die();
                break;
            }
        }
    }

    nmes = players.size();
}

sf::Color World::getAmbientLight() const {
    float t = static_cast<float>(time % DAY_LENGTH) / DAY_LENGTH; // 0..1

    float brightness = 0.25f + 0.75f * std::sin(t * 3.14159f);

    sf::Color nightColor(150, 180, 255); // night tint
    sf::Color dayColor(255, 255, 255); // day tint

    // sigmoid-like smoothstep around dawn/dusk
    float factor;
    if (t < 0.25f) {
        factor = t / 0.25f; // 0..1 from midnight to dawn
    } else if (t > 0.75f) {
        factor = (1.f - t) / 0.25f; // 0..1 from dusk to midnight
    } else {
        factor = 1.f; // day time
    }
    factor = std::clamp(factor, 0.f, 1.f);
    uint8_t r = static_cast<uint8_t>(brightness * (nightColor.r * (1 - factor) + dayColor.r * factor));
    uint8_t g = static_cast<uint8_t>(brightness * (nightColor.g * (1 - factor) + dayColor.g * factor));
    uint8_t b = static_cast<uint8_t>(brightness * (nightColor.b * (1 - factor) + dayColor.b * factor));

    return sf::Color(r, g, b);
}


void World::save_dirty_chunks() {
    for (auto& c : chunk) {
        if (c->isDirty()) {
            c->save();
            c->tidy();
        }
    }
}