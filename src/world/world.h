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
    void addLight(LightSource light);
    Entity* getPlayer();
    Player* getNearestEntity(Entity* from);
    bool isSolidAt(sf::Vector2f pos, sf::Vector2f size) const;
    sf::Color getAmbientLight() const;
    sf::String getTimeOfDay() const;
    sf::Vector2i worldToTileCoords(sf::Vector2f const& pos) const;
    bool isValidTile(sf::Vector2i const& tile) const;
public:
    std::string const& getName() { return name; }

public:
    void update(sf::Time dt);
    void update_tick(sf::Time elapsed);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
private:
    void spawn_loot();
    void check_collisions();
    void propagate_light(const LightSource& light, std::vector<std::vector<LightTile>>& grid,const sf::Vector2i& viewTileOffset, int worldWidthTiles, int worldHeightTiles) const;
    sf::Sprite getTileLightSprite(float intensity, float radius) const;
    void save_dirty_chunks();
private:
    std::vector<std::unique_ptr<Entity>> entities{};

    std::vector<LightSource> lights;
    sf::Texture light_tex;
    mutable sf::RenderTexture lightmap;

    Entity* player;
    sf::Clock chunk_idle{};
    std::vector<Chunk*> chunk;
    std::string name;

    // daynight cycle
    uint64_t time = 20 * 60 * 0;
    static constexpr uint64_t DAY_LENGTH = 60 * 20 * 60; // 60 minute for now
    bool pause_time = true;
    std::vector<std::vector<LightTile>> tile_light_grid;
    std::vector<std::vector<bool>> dirty_tiles;

    // test minigame vars:
    int nmes;
    bool ctrl_dead = false;
    bool restart_shoot = false;
    sf::Clock grace{};
};