#include "world.h"

#include "../entity/players.h"
#include "../entity/arrow.h"
#include "../item/sword.h"
#include "../entity/itemstack.h"
#include "../game.h"
#include "../net.h"

#include <algorithm>
#include <random>
#include <queue>
#include <cmath>

namespace {
    // inline bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b) {
    //     const float aLeft   = a.position.x;
    //     const float aTop    = a.position.y;
    //     const float aRight  = aLeft + a.size.x;
    //     const float aBottom = aTop  + a.size.y;

    //     const float bLeft   = b.position.x;
    //     const float bTop    = b.position.y;
    //     const float bRight  = bLeft + b.size.x;
    //     const float bBottom = bTop  + b.size.y;

    //     // strict comparisons: touching edges do NOT count as a hit
    //     return (aLeft < bRight) && (aRight > bLeft) && (aTop < bBottom) && (aBottom > bTop);
    //     // if you want “touching counts”, switch < and > to <= and >=.
    // }

    inline bool overlaps(const sf::FloatRect& a, const sf::FloatRect& b) {
        return !(a.position.x + a.size.x  <= b.position.x  ||
                 b.position.x + b.size.x <= a.position.x  ||
                 a.position.y  + a.size.y <= b.position.y  ||
                 b.position.y + b.size.y <= a.position.y);
    }

    inline int fdiv(int a, int b) {
        int q = a / b;
        int r = a % b;
        if ((r != 0) && ((a < 0) != (b < 0))) {
            q--; // round toward -∞
        }
        return q;
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


World::World(std::string const& name) : name(name), tile_highlight(Game::getInstance()->getFallbackFont()) {
    entities.reserve(1000); // TODO FIX: this is a temporary solution I need to fully move to Id's instead of ptrs and weak ptrs
    light_tex = ::generateRadialGradient(256);
    lightmap = sf::RenderTexture(Game::getInstance()->getWindow().getSize());

    // spawn_loot();
    rebakeLighting();
}

World::~World() {

}

void World::addEntity(std::unique_ptr<Entity> entity) {
    if (dynamic_cast<ControllingPlayer*>(entity.get()))
        player = dynamic_cast<Player*>(entity.get());

    entities.push_back(std::move(entity));
}

Player* World::getPlayer() {
    return player;
}

sf::Vector2i World::worldToTileCoords(const sf::Vector2f& pos) const {
    return sf::Vector2i(
        static_cast<int>(std::floor(pos.x / TILE_SIZE)),
        static_cast<int>(std::floor(pos.y / TILE_SIZE))
    );
}

sf::Vector2f World::tileToWorldCoords(const sf::Vector2i& tile) const {
    return sf::Vector2f{
        static_cast<float>(tile.x * TILE_SIZE),
        static_cast<float>(tile.y * TILE_SIZE)
    };
}


bool World::isValidTile(const sf::Vector2i& tile) const {
    for (auto& c : chunks) {
        int col = tile.x - static_cast<int>(c.second->getOffset().x / TILE_SIZE);
        int row = tile.y - static_cast<int>(c.second->getOffset().y / TILE_SIZE);
        if (row >= 0 && row < CHUNK_SIZE && col >= 0 && col < CHUNK_SIZE)
            return true;
    }
    return false;
}

bool World::isSolidTile(sf::Vector2i const& pos) const {
    for (auto& c : chunks) {
        sf::Vector2i local{
            pos.x - static_cast<int>(c.second->getOffset().x / TILE_SIZE),
            pos.y - static_cast<int>(c.second->getOffset().y / TILE_SIZE)
        };

        if (local.x >= 0 && local.x < CHUNK_SIZE &&
            local.y >= 0 && local.y < CHUNK_SIZE) {
            return c.second->isSolidTile(local.y, local.x);
        }
    }
    return false;
}

void World::burstSmoke(sf::Vector2f const& wcoords, int count, bool left) {
    smoke.spawnBurst(wcoords, count, left);
}


bool World::isPassableAt(sf::Vector2f pos, sf::Vector2f size) const {
    // determine the AABB corners of the entity
    sf::Vector2f min = pos - size * 0.5f;
    sf::Vector2f max = pos + size * 0.5f;

    int min_tx = static_cast<int>(std::floor(min.x / TILE_SIZE));
    int max_tx = static_cast<int>(std::floor(max.x / TILE_SIZE));
    int min_ty = static_cast<int>(std::floor(min.y / TILE_SIZE));
    int max_ty = static_cast<int>(std::floor(max.y / TILE_SIZE));

    for (int ty = min_ty; ty <= max_ty; ++ty) {
        for (int tx = min_tx; tx <= max_tx; ++tx) {
            sf::Vector2i tile{tx, ty};
            bool passable = false;

            // Find the chunk containing this tile
            for (auto& c : chunks) {
                sf::Vector2i local{
                    tile.x - static_cast<int>(c.second->getOffset().x / TILE_SIZE),
                    tile.y - static_cast<int>(c.second->getOffset().y / TILE_SIZE)
                };

                if (local.x >= 0 && local.x < CHUNK_SIZE &&
                    local.y >= 0 && local.y < CHUNK_SIZE) {
                    passable = c.second->isPassableTile(local.y, local.x);
                    break; // found the chunk
                }
            }

            if (!passable) return false; // any blocked tile blocks the entity
        }
    }

    return true; // all tiles passable
}



Player* World::getNearestEntity(Entity* from) {
    if (!from) return nullptr;

    Player* nearest = nullptr;
    float min_dist = 1000.f;
    float minDistanceSq = min_dist * min_dist; // use squared distance to avoid sqrt

    for (auto& e : entities) {
        if (!e || !e->isAlive() || e.get() == from || e->isInvincible())
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

std::optional<sf::Vector2f> World::raycast(sf::Vector2f start, sf::Vector2f dir, float maxDist) const {
    // Normalize direction
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f) return std::nullopt;
    sf::Vector2f ndir = dir / len;

    // Step size in world units
    float step = TILE_SIZE / 4.f; // finer steps = more precise
    sf::Vector2f pos = start;

    float dist = 0.f;
    while (dist < maxDist) {
        sf::Vector2i tile = worldToTileCoords(pos);
        if (isSolidTile(tile)) {
            return pos; // hit!
        }

        pos += ndir * step;
        dist += step;
    }
    return std::nullopt; // no hit
}

sf::Vector2i World::getPlayerChunk() const {
    if (!player) return {0, 0};
    sf::Vector2f pos = player->getPosition();
    return {
        static_cast<int>(std::floor(pos.x / (CHUNK_SIZE * TILE_SIZE))),
        static_cast<int>(std::floor(pos.y / (CHUNK_SIZE * TILE_SIZE)))
    };
}

void World::update_chunks() {
    constexpr int CHUNK_RADIUS = 2;
    if (!player) return;

    sf::Vector2i playerChunk = getPlayerChunk();
    std::unordered_set<sf::Vector2i, arb::Vector2iHash> desired;

    // 1. build desired 3x3 chunk region
    for (int dy = -CHUNK_RADIUS; dy <= CHUNK_RADIUS; ++dy) {
        for (int dx = -CHUNK_RADIUS; dx <= CHUNK_RADIUS; ++dx) {
            desired.insert({playerChunk.x + dx, playerChunk.y + dy});
        }
    }

    // 2. unload chunks that are far away
    std::vector<sf::Vector2i> to_remove;
    for (auto& [pos, chunk] : chunks) {
        if (desired.find(pos) == desired.end()) {
            to_remove.push_back(pos);
        }
    }

    for (auto& pos : to_remove) {
        chunks.erase(pos);
    }

    // 3. request missing chunks
    auto network = ClientNetwork::getInstance();

    for (auto& pos : desired) {
        if (chunks.find(pos) == chunks.end()) {
            network->requestChunk(pos);
        }
    }
}
void World::addChunk(std::unique_ptr<Chunk> chunk) {
    sf::Vector2i pos = chunk->getPos();
    chunks[pos] = std::move(chunk);
}

void World::update(sf::Time dt) {
    if (!player || !player->isAlive()) {

    }
    tile_highlight.update(Game::getInstance()->getMouseWorld());

    static sf::Clock chunkStreamTimer;
    if (chunkStreamTimer.getElapsedTime() > sf::seconds(0.5f)) { // twice a second
        update_chunks();
        chunkStreamTimer.restart();
    }

    auto network = ClientNetwork::getInstance();
    // spawn new RemotePlayers
    for (auto const& [id, rp] : network->getPlayers()) {
        if (id == network->getMyId()) continue;
        if (remote_players.find(id) == remote_players.end()) {
            auto player = std::make_unique<RemotePlayer>(rp.state);
            addRemote(id, std::move(player));
        }
    }

    // despawn removed RemotePlayers
    // std::vector<uint32_t> to_remove;
    // for (auto const& [id, player] : remote_players) {
    //     if (network->getPlayers().find(id) == network->getPlayers().end()) {
    //         to_remove.push_back(id);
    //     }
    // }
    // for (auto id : to_remove) removeRemote(id);

    // update targets (no interpolation here, interpolation happens per tick)
    for (auto& [id, player] : remote_players) {
        auto it = network->getPlayers().find(id);
        if (it != network->getPlayers().end()) {
            player->setTarget(it->second.state.pos, it->second.state.vel, it->second.state.rotation, it->second.state.held_item); // latest server snapshot
        }
    }

    for (auto& x : entities) {
        if (x && x->isAlive())
            x->update(dt);
    }

    smoke.update(dt.asSeconds());

    check_collisions();
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](auto& e){ return !e->isAlive(); }),
        entities.end()
    );
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

void World::setTime(uint64_t time) {
    this->time = time % DAY_LENGTH;
}

void World::update_tick(sf::Time elapsed) {
    for (auto& x : entities) {
        if (x && x->isAlive()) {
            x->update_tick(elapsed);
        }
    }

    ClientNetwork::getInstance()->syncInput(player->getVelocity(), player->getItemRotation(), (uint16_t) player->getHoldingType());

    static Tile pref;
    bool focus = Game::getInstance()->shouldWorldFocus();
    if (focus) {
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
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5)) {
            pref = Tile::Sand;
        }
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && focus) {
        sf::Vector2f mouse = Game::getInstance()->getMouseWorld();

        for (auto& c : chunks) {
            sf::Vector2f local = mouse - c.second->getOffset();

            // Only update if the mouse is *inside this chunk*
            if (local.x >= 0.f && local.x < CHUNK_SIZE * TILE_SIZE &&
                local.y >= 0.f && local.y < CHUNK_SIZE * TILE_SIZE) {
                auto cX = static_cast<uint32_t>(local.x / TILE_SIZE);
                auto cY = static_cast<uint32_t>(local.y / TILE_SIZE);

                ClientNetwork::getInstance()->submitTile(c.second->getPos(), {cX, cY}, pref);
                break; // stop after the first matching chunk
            }
        }
        rebakeLighting();
    }


    if (chunk_idle.getElapsedTime() >= sf::seconds(2.f)) {
        save_dirty_chunks();
        chunk_idle.restart();
    }
}

Chunk* World::getChunk(sf::Vector2i const& pos) {
    if (chunks.find(pos) != chunks.end()) {
        return chunks.at(pos).get();
    }
    return nullptr;
}

TileRef World::resolve(sf::Vector2i const& wtile) const {
    int cx = ::fdiv(wtile.x, CHUNK_SIZE);
    int cy = ::fdiv(wtile.y, CHUNK_SIZE);
    if (wtile.x < 0 && wtile.x % CHUNK_SIZE != 0) cx--;
    if (wtile.y < 0 && wtile.y % CHUNK_SIZE != 0) cy--;

    int lx = wtile.x - cx * CHUNK_SIZE;
    int ly = wtile.y - cy * CHUNK_SIZE;


    for (auto& c: chunks) {
        if (c.second->getPos() == sf::Vector2i{cx, cy}) {
            TileRef r;
            r.lx = lx;
            r.ly= ly;
            r.chunk = c.second.get();
            return r;
        }
    }
    return {nullptr, 0, 0};
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

    for (auto& c: chunks) {
        if (::overlaps(viewRect, c.second->getBounds())) {
            c.second->render(target);
        }
    }

    for (auto& x : entities) {
        if (!x) continue;

        if (::overlaps(viewRect, x->getBounds())) {
            target.draw(*x, states);
        }
    }

    draw_lighting(target, viewRect);
    target.draw(smoke, states);

    target.draw(tile_highlight);
}

void World::draw_lighting(sf::RenderTarget& target, sf::FloatRect const& viewRect) const {
    lightmap.setView(target.getView());
    lightmap.clear(getAmbientLight());

    sf::VertexArray lightVerts{sf::PrimitiveType::Triangles};
    for (auto const& [pos, lt] : lightmap_tiles) {
        if (lt.intensity <= 0.f) continue;

        float x = pos.x * TILE_SIZE;
        float y = pos.y * TILE_SIZE;

        // Cull tile if outside viewRect
        sf::FloatRect tileBounds({x, y}, {TILE_SIZE, TILE_SIZE});
        if (!::overlaps(viewRect, tileBounds))
            continue;

        sf::Color c(255, 255, 200, std::clamp((int)(lt.intensity * 20.f), 0, 255));

        // Two triangles per tile (6 vertices)
        lightVerts.append({{x, y}, c});
        lightVerts.append({{x + TILE_SIZE, y}, c});
        lightVerts.append({{x + TILE_SIZE, y + TILE_SIZE}, c});

        lightVerts.append({{x, y}, c});
        lightVerts.append({{x + TILE_SIZE, y + TILE_SIZE}, c});
        lightVerts.append({{x, y + TILE_SIZE}, c});
    }

    lightmap.draw(lightVerts, sf::BlendAdd);
    lightmap.display();

    sf::Sprite lm_sprite{lightmap.getTexture()};
    lm_sprite.setPosition(target.mapPixelToCoords({0, 0}));
    lm_sprite.setScale({
        target.getView().getSize().x / static_cast<float>(lightmap.getSize().x),
        target.getView().getSize().y / static_cast<float>(lightmap.getSize().y)
    });

    target.draw(lm_sprite, sf::BlendMultiply);
}


void World::spawn_loot() {
    std::vector<sf::Vector2f> candid;

    for (auto& c : chunks) {
        for (int row = 0; row < CHUNK_SIZE; ++row) {
            for (int col = 0; col < CHUNK_SIZE; ++col) {
                Tile t = c.second->getTile(row, col);
                if (t != Tile::Water && t != Tile::Cobble) {
                    // tile center in world space
                    sf::Vector2f pos = c.second->getOffset() +
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
    int num = std::min(50, (int) candid.size());

    for (int i = 0; i < num; ++i) {
        sf::Vector2f spawn = candid[i];
        addEntity(std::make_unique<ItemStack>(static_cast<ItemType>(rand() % 16), spawn));
    }
}

void World::addLight(sf::Vector2i at, float r, sf::Color col) {
    TileLightSource l;
    l.color = col;
    l.position = at;
    l.radius = r;
    lights.push_back(l);

    rebakeLighting();
}

void World::rebakeLighting() {
    lightmap_tiles.clear();

    std::vector<std::pair<sf::Vector2i,float>> lightsources;

    for (auto& c : chunks) {
        std::vector<StaticObject*> objectLights;
        c.second->collectLights(objectLights);
        for (auto* o : objectLights) {
            sf::Vector2i worldTile = o->origin + sf::Vector2i{
                static_cast<int>(c.second->getOffset().x / TILE_SIZE),
                static_cast<int>(c.second->getOffset().y / TILE_SIZE)
            };

            lightsources.push_back({worldTile, o->light_radius});
        }
    }

    for (auto const& src : lights) {
        lightsources.push_back({src.position, src.radius});
    }

    for (auto const& [tile, r] : lightsources) {
        std::queue<std::pair<sf::Vector2i, float>> q;
        std::unordered_map<sf::Vector2i, float, arb::Vector2iHash> visited;

        q.push({tile, r});
        visited[tile] = r;

        while (!q.empty()) {
            auto [pos, strength] = q.front();
            q.pop();

            if (!isValidTile(pos)) continue;
            if (strength <= 0.f) continue;

            // illuminate this tile
            auto& lt = lightmap_tiles[pos];
            lt.intensity = std::max(lt.intensity, strength);

            float decay = 1.f;
            float next_strength = strength - decay;
            if (next_strength <= 0.f) continue;

            for (sf::Vector2i dir : {sf::Vector2i{1,0}, {-1,0}, {0,1}, {0,-1}}) {
                sf::Vector2i next = pos + dir;
                if (!isValidTile(next)) continue;

                if (isSolidTile(next)) {
                    // illuminate solid tile
                    auto& wallLt = lightmap_tiles[next];
                    wallLt.intensity = std::max(wallLt.intensity, next_strength);

                    // also illuminate *adjacent* solid tiles (so entire wall gets lit)
                    for (sf::Vector2i adjDir : {sf::Vector2i{1,0}, {-1,0}, {0,1}, {0,-1}}) {
                        sf::Vector2i adj = next + adjDir;
                        if (!isValidTile(adj)) continue;
                        if (isSolidTile(adj)) {
                            auto& adjLt = lightmap_tiles[adj];
                            adjLt.intensity = std::max(adjLt.intensity, next_strength - 0.5f);
                        }
                    }

                    // don’t enqueue solid and stop propagation
                    continue;
                }

                // passable tile is enqueued normally
                if (visited.find(next) == visited.end() || visited[next] < next_strength) {
                    visited[next] = next_strength;
                    q.push({next, next_strength});
                }
            }
        }
    }
}

void World::check_collisions() {
    std::vector<Arrow*> arrows;
    std::vector<ItemStack*> items;
    std::vector<Player*> players;

    // Preallocate to avoid reallocations
    arrows.reserve(entities.size());
    items.reserve(entities.size());
    players.reserve(entities.size());

    // --- classify entities (fast, no dynamic_cast) ---
    for (auto& uptr : entities) {
        if (!uptr || !uptr->isAlive())
            continue;

        switch (uptr->getType()) {
            case EntityType::ArrowEntity:
                arrows.push_back(static_cast<Arrow*>(uptr.get()));
                break;
            case EntityType::ItemEntity:
                items.push_back(static_cast<ItemStack*>(uptr.get()));
                break;
            case EntityType::PlayerEntity:
                players.push_back(static_cast<Player*>(uptr.get()));
                break;
            default: break;
        }
    }

    // --- arrows vs players ---
    for (Arrow* arrow : arrows) {
        const EntityId shooter = arrow->getShooterId();
        const sf::FloatRect& ab = arrow->getBounds();

        for (Player* p : players) {
            if (p->getId() == shooter) continue;
            if (::overlaps(ab, p->getBounds())) {
                p->damage(10.f);
                if (dynamic_cast<ControllingPlayer*>(p) && !p->isAlive()) {

                }
                arrow->die();
                break;
            }
        }
    }

    // --- items vs players ---
    for (ItemStack* item : items) {
        const sf::FloatRect& ib = item->getBounds();
        for (Player* p : players) {
            if (::overlaps(ib, p->getBounds())) {
                p->pickup(item->getType());
                item->die();
                break; // item consumed
            }
        }
    }
}

void World::addRemote(uint32_t id, std::unique_ptr<RemotePlayer> player) {
    RemotePlayer* ptr = player.get();
    remote_players[id] = ptr;
    entities.push_back(std::move(player));
}

void World::removeRemote(uint32_t id) {
    auto it = remote_players.find(id);
    if (it != remote_players.end()) {
        RemotePlayer* victim = it->second;
        entities.erase(
            std::remove_if(
                entities.begin(),
                entities.end(),
                [&](const std::unique_ptr<Entity>& e) {
                    return e.get() == victim;
                }),
            entities.end());

        remote_players.erase(it);
    }
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

}