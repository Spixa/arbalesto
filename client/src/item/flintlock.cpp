#include "flintlock.h"
#include "../game.h"

void Flintlock::onLMB(sf::Vector2f const& dir, Entity* user) {
    auto w = Game::getInstance()->getWorld();
    auto muzzle = user->getPosition();
    auto hit = w->raycast(muzzle, dir, 5000.f);

    auto r = rand();
    auto rd = r % 5;
    float diff = 0.1f * (rd - 2);
    shooting_sound.setPitch(1.f + diff);
    shooting_sound.play();

    w->burstSmoke(user->getPosition() + dir * 8.f, 40, dir.x > 0);
    if (hit) {
        w->burstSmoke(*hit, 25.f, dir.x > 0);
    }
}

void Flintlock::update_derived(sf::Time dt, bool facing, sf::Vector2f const& dir) {
    sf::Angle rotation = sf::radians(std::atan2(dir.y, dir.x));
    setRotation(rotation);
}