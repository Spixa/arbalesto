#pragma once

#include <SFML/Graphics.hpp>

#include "../entity/entity.h"
#include "chunk.h"
#include "lighting.h"

class Player;
class World : public sf::Drawable, public sf::Transformable {
public:
    World(std::string const& name);
    virtual ~World();

    void addEntity(std::unique_ptr<Entity> entity);
    void addLight(sf::Vector2i at, float r, sf::Color col);
    Entity* getPlayer();
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
public:
    std::string const& getName() { return name; }

public:
    void update(sf::Time dt);
    void update_tick(sf::Time elapsed);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    void draw_lighting(sf::RenderTarget&) const;
private:
    void spawn_loot(); // expensive
    void check_collisions(); // optimized
    void save_dirty_chunks(); // expensive
private:
    std::vector<std::unique_ptr<Entity>> entities{};

    // lighting
    std::vector<TileLightSource> lights;
    sf::Texture light_tex;
    mutable sf::RenderTexture lightmap; // because ::draw() is const
    std::unordered_map<sf::Vector2i, LightTile, arb::Vector2iHash> lightmap_tiles;

    Entity* player;
    sf::RectangleShape tile_highlight;
    sf::Clock chunk_idle{};
    std::vector<Chunk*> chunk;
    std::string name;

    // daynight cycle
    uint64_t time = 20 * 60 * 0;
    static constexpr uint64_t DAY_LENGTH = 60 * 20 * 60; // 60 minute for now
    bool pause_time = false;

    // test minigame vars:
    int nmes;
    bool ctrl_dead = false;
    bool restart_shoot = false;
    sf::Clock grace{};
};