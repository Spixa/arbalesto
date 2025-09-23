#include "smokesystem.h"
#include <cmath>

SmokeSystem::SmokeSystem()
: verts(sf::PrimitiveType::Triangles),
  rng(std::random_device{}()),
  angleDist(0.f, 2.f * 3.1415926f),
  speedDist(5.f, 45.f),       // gentle sideways push
  sizeDist(3.f, 12.f),        // starting puff size
  lifeDist(0.5f, 3.f)         // seconds of life
{}

void SmokeSystem::spawnBurst(sf::Vector2f origin, int count, bool left) {
    for (int i = 0; i < count; i++) {
        float angle = angleDist(rng);
        float speed = speedDist(rng);

        SmokeParticle p;
        p.pos = origin;
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed - 30.f}; 
        // "-30.f" gives natural upward drift

        p.size = sizeDist(rng);
        p.life = p.maxLife = lifeDist(rng);
        p.alpha = 255.f;
        p.alive = true;
        p.left = left;
        particles.push_back(p);
    }
}

void SmokeSystem::update(float dt) {
    for (auto& p : particles) {
        if (!p.alive) continue;

        p.life -= dt;
        if (p.life <= 0.f) {
            p.alive = false;
            continue;
        }
        p.pos += p.vel * dt;
        p.vel.y -= 50.f * dt;

        float driftSpeed = 100.f * dt;
        p.vel.x += p.left ? -driftSpeed : driftSpeed;

        p.size += dt * 15.f;

        p.alpha = 255.f * (p.life / p.maxLife);
    }
}

void SmokeSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    verts.clear();

    for (auto const& p : particles) {
        if (!p.alive) continue;

        sf::Color c(180, 180, 180, static_cast<uint8_t>(p.alpha));

        float s = p.size;
        float x = p.pos.x;
        float y = p.pos.y;

        // draw as a quad (two triangles)
        verts.append({{x, y}, c});
        verts.append({{x + s, y}, c});
        verts.append({{x + s, y + s}, c});

        verts.append({{x, y}, c});
        verts.append({{x + s, y + s}, c});
        verts.append({{x, y + s}, c});
    }
    states.blendMode = sf::BlendAlpha;
    target.draw(verts, states);
}
