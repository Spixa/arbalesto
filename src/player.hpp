#pragma once
#include "sprite.hpp"
#include "animation.hpp"
#include "game.hpp"

struct PlayerSprites {
    sf::Sprite* body;
    sf::Sprite* hand;
    sf::Sprite* feet;
};

enum class AttackState {
    Bow,
    Sword,
    Fist = 0
};

class Player : public HitboxSprite {
public:
    Player()
        : anim(0.1, this)
    {
        setScale({2.0, 2.0});
        setHitbox({0, 0, 32, 32});

        state = AttackState::Fist;
    }

    void update(sf::Time deltaTime) {
        float dt = deltaTime.asSeconds();
        constexpr float speed = 256.f;

        sf::Vector2f velo = {
            speed * dt * (sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A)),
            speed * dt * (sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        };

        move(velo);

        if (velo.x < 0) inv = true;
        else if (velo.x > 0) inv = false;
        else {}

        if (velo.x != 0 || velo.y != 0) anim.update(1, deltaTime, inv, state);
        else anim.update(0, deltaTime, inv, state);
    
    }

    PlayerSprites getSprites() {
        return { &bodySprite, &handsSprite, &feetSprite};
    }
    
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();

        target.draw(feetSprite, states);
        target.draw(bodySprite, states);
        target.draw(handsSprite, states);
    }
private:
    sf::Sprite bodySprite;
    sf::Sprite handsSprite;
    sf::Sprite feetSprite;
    bool inv;

    PlayerAnimation anim;
    AttackState state;
};