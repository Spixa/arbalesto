#pragma once

#include <SFML/Graphics.hpp>
#include "../entity/entity.h"
#include "chunk.h"

class Player;
class World : public sf::Drawable, public sf::Transformable {
public:
    World(std::string const& name);
    virtual ~World();

    void addEntity(std::unique_ptr<Entity> entity);
    Entity* getPlayer();
    Player* getNearestEntity(Entity* from);
    bool isSolidAt(sf::Vector2f pos, sf::Vector2f size) const;

public:
    std::string const& getName() { return name; }

public:
    void update(sf::Time dt);
    void update_tick(sf::Time elapsed);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    void check_arrow_collisions();
private:
    std::vector<std::unique_ptr<Entity>> entities{};
    Entity* player;
    std::vector<Chunk*> chunk;
    std::string name;

    // test minigame vars:
    int nmes;
    bool ctrl_dead = false;
};