#include "bow.h"
#include "../entity/arrow.h"
#include "../entity/entity.h"

void Bow::onLMB(sf::Vector2f const& dir, Entity* user) {
    auto arrow_ptr = std::make_unique<Arrow>(user->getPosition(), dir, 300.f, sf::seconds(5.f), user->getId());
    Game::getInstance()->getWorld()->addEntity(std::unique_ptr<Entity>(std::move(arrow_ptr)));
}

void Bow::update_derived(sf::Time dt, bool facing, sf::Vector2f const& dir) {
    sf::Angle rotation = sf::radians(std::atan2(dir.y, dir.x)) + sf::degrees(-135);
    setRotation(rotation);
}