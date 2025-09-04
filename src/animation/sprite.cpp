#include "sprite.h"
#include "../game.h"

AnimatedSprite::AnimatedSprite(std::string const& prefix, sf::Vector2u dims, float switch_time) : image_count(dims), switch_time(switch_time) {
    idle_texture = Game::getInstance()->getTextureManager().get(prefix + "_idle");
    walk_texture = Game::getInstance()->getTextureManager().get(prefix + "_walk");
    sprite.setSize({16,16});
    sprite.setOrigin(sprite.getSize() / 2.f);
    sprite.setTexture(idle_texture.get());

    sf::Vector2u tsize = idle_texture->getSize();
    total_time = 0.0f;
    anim_rect.size.x = tsize.x / float(image_count.x);
    anim_rect.size.y = tsize.y / float(image_count.y);
}

AnimatedSprite::~AnimatedSprite() {

}

void AnimatedSprite::update(sf::Time dt, int row, bool invert) {
    float dt_s = dt.asSeconds();

    current_image.y = row;
    total_time += dt_s;

    if (total_time >= switch_time) {
        total_time -= switch_time;
        current_image.x += 1;

        if (current_image.x >= image_count.x) {
            current_image.x = 0;
        }
    }

    anim_rect.position.x = current_image.x * anim_rect.size.x;
    anim_rect.position.y = current_image.y * anim_rect.size.y;

    if (!invert) {
        sprite.setTextureRect(anim_rect);
    } else {
        sprite.setTextureRect(sf::IntRect({
            static_cast<int>(anim_rect.position.x + anim_rect.size.x),
            static_cast<int>(anim_rect.position.y)},
            {-static_cast<int>(anim_rect.size.x),
            static_cast<int>(anim_rect.size.y)}
        ));
    }
}

void AnimatedSprite::setWalking(bool walking) {
    if (walking) {
        sprite.setTexture(walk_texture.get());
    } else {
        sprite.setTexture(idle_texture.get());
    }
}

void AnimatedSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();

    target.draw(sprite, states);
}