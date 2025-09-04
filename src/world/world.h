#pragma once

#include <SFML/Graphics.hpp>
#include "../entity/entity.h"
#include "chunk.h"

class ControllingPlayer;
class AiPlayer;
class World : public sf::Drawable, public sf::Transformable {
public:
    World(std::string const& name);
    virtual ~World();

    void addEntity(std::unique_ptr<Entity> entity);
    ControllingPlayer* getPlayer();
    AiPlayer* getNearestAiPlayer(Entity* from);
    std::string const& getName() { return name; }
    void update(sf::Time dt);
    void update_tick(sf::Time elapsed);
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
    void check_arrow_collisions();
private:
    std::vector<std::unique_ptr<Entity>> entities{};
    ControllingPlayer* player;
    std::vector<Chunk*> chunk;
    std::string name;
};