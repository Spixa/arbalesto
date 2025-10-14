#pragma once

#include <SFML/Graphics.hpp>

#include "../entity/entity.h"
#include "chunk.h"
#include "lighting.h"
#include "../particle/smokesystem.h"

class Player;
class RemotePlayer;
class World : public sf::Drawable, public sf::Transformable {
public:
    World(std::string const& name);
    virtual ~World();

    void addEntity(std::unique_ptr<Entity> entity);
    void addLight(sf::Vector2i at, float r, sf::Color col);
    void burstSmoke(sf::Vector2f const& wcoords, int count, bool left);
    Player* getPlayer();
    sf::Vector2i getPlayerChunk() const;
    Player* getNearestEntity(Entity* from);
    bool isPassableAt(sf::Vector2f pos, sf::Vector2f size) const;
    sf::Color getAmbientLight() const;
    sf::Vector2i worldToTileCoords(const sf::Vector2f& pos) const;
    sf::Vector2f tileToWorldCoords(const sf::Vector2i& pos) const;
    bool isValidTile(const sf::Vector2i& tile) const;
    bool isSolidTile(sf::Vector2i const& pos) const;
    sf::String getTimeOfDay() const;

    void rebakeLighting(); // expensive
    TileRef resolve(sf::Vector2i const& wtile) const;
    std::optional<sf::Vector2f> raycast(sf::Vector2f start, sf::Vector2f dir, float maxDist) const;

    std::vector<std::unique_ptr<Entity>>& getEntities();

    void addRemote(uint32_t id, std::unique_ptr<RemotePlayer> player);
    void addChunk(std::unique_ptr<Chunk> chunk);
    Chunk* getChunk(sf::Vector2i const& pos);
    void removeRemote(uint32_t id);
    void setTime(uint64_t time);
public:
    std::string const& getName() { return name; }

public:
    void update(sf::Time dt);
    void update_tick(sf::Time elapsed);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    void draw_lighting(sf::RenderTarget&, sf::FloatRect const& viewRect) const;
private:
    void spawn_loot(); // expensive
    void update_chunks(); // expensive
    void check_collisions(); // optimized
    void save_dirty_chunks(); // expensive
private:
    std::vector<std::unique_ptr<Entity>> entities{};
    std::unordered_map<uint32_t, RemotePlayer*> remote_players;

    // lighting
    std::vector<TileLightSource> lights;
    sf::Texture light_tex;
    mutable sf::RenderTexture lightmap; // because ::draw() is const
    std::unordered_map<sf::Vector2i, LightTile, arb::Vector2iHash> lightmap_tiles;

    Player* player;
    SmokeSystem smoke;
    TileHighlight tile_highlight;
    sf::Clock chunk_idle{};
    std::unordered_map<sf::Vector2i, std::unique_ptr<Chunk>, arb::Vector2iHash> chunks;
    std::string name;

    // daynight cycle
    uint64_t time = 20 * 60 * 0;
    static constexpr uint64_t DAY_LENGTH = 60 * 20 * 60; // 60 minute for now
    bool pause_time = false;
};