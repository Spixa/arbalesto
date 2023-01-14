#include "animation.hpp"
#include "player.hpp"

PlayerAnimation::PlayerAnimation(float switchTime, Player* player) :
    body("../res/textures/body.png", {6, 8}, switchTime),
    hand("../res/textures/hand.png", {6, 8}, switchTime),
    feet("../res/textures/feet.png", {6, 8}, switchTime)
{   
    body.setCurrentImageX(0);
    hand.setCurrentImageX(0);
    this->player = player;

    player->getSprites().body->setTexture(body.getTexture());
    player->getSprites().hand->setTexture(hand.getTexture());
    player->getSprites().feet->setTexture(feet.getTexture());
}

void PlayerAnimation::update(int row, sf::Time deltaTime, bool i, AttackState state)
{
    float dt = deltaTime.asSeconds();

    if (state == AttackState::Fist) {
        body.update(row, dt);
        hand.update(row, dt);
    } else {
        body.update(2, dt);
        hand.update(2, dt);
    }

    feet.update(row, dt);

    auto b = body.getAnimRect();
    auto h = hand.getAnimRect();
    auto f = feet.getAnimRect();

    player->getSprites().body->setTextureRect(b);
    player->getSprites().hand->setTextureRect(h);
    player->getSprites().feet->setTextureRect(f);
}
