#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

struct SmokeParticle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float size;
    float life;       // remaining time
    float maxLife;    // original lifespan
    float alpha;      // current opacity
    bool alive;
    bool left;
};

class SmokeSystem : public sf::Drawable, public sf::Transformable {
public:
    SmokeSystem();

    void spawnBurst(sf::Vector2f origin, int count, bool left);
    void update(float dt);
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates) const override;
private:
    std::vector<SmokeParticle> particles;
    mutable sf::VertexArray verts;

    std::mt19937 rng;
    std::uniform_real_distribution<float> angleDist;
    std::uniform_real_distribution<float> speedDist;
    std::uniform_real_distribution<float> sizeDist;
    std::uniform_real_distribution<float> lifeDist;
};
