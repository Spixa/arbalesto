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
    BowAttack,
    BowHold,
    SwordAttack,
    SwordHold,
    Fist = 0
};

class Player : public HitboxSprite {
public:
    Player(std::string const& displayName = "", sf::Vector2f const& startingPos = {0.f, 0.f})
        : anim(0.1, this), displayName(displayName)
    {
        setPosition(startingPos);
        setHitbox({0, 0, 32, 32});

        
        displayNameFont.loadFromFile("../res/fonts/main.ttf");
        displayNameText.setFont(displayNameFont);
        displayNameText.setCharacterSize(12.f);
        displayNameText.setString(displayName);
        displayNameText.setPosition(0.f, -15.f);
    }

    bool isControlled() const { return controlled; }
    void setControlled(bool tof) { 
        controlled = tof; 
    
        if (controlled) {
            displayNameText.setFillColor(sf::Color::Green);
        } else {
            displayNameText.setFillColor(sf::Color::White);
        }
    }

    void update(sf::Time deltaTime) {
        float dt = deltaTime.asSeconds();

        // Legit speed: 256.f
        constexpr float speed = 256 * 1.f;

        if (controlled && Game::getInstance()->isFocused()) {
            velo = {
                speed * dt * (sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A)),
                speed * dt * (sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            };
            

            if (state == AttackState::Fist && sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                state = AttackState::SwordHold;
                L("Player is now holding Sword");
            } else if ((state == AttackState::SwordAttack || state == AttackState::SwordHold) && sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                state = AttackState::Fist;
                L("Player is now holding FIST");
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && state == AttackState::SwordHold) {
                state = AttackState::SwordAttack;
            }

            if (attackTimer.getElapsedTime().asSeconds() >= 0.8 && state == AttackState::SwordAttack) {
                state = AttackState::SwordHold;
                attackTimer.restart();
            }
        }

        move(velo);

        if (velo.x < 0) inv = true;
        else if (velo.x > 0) inv = false;
        else {}

        if (velo.x != 0 || velo.y != 0) { anim.update(1, deltaTime, inv, state); moving = true; }
        else { anim.update(0, deltaTime, inv, state); moving = false; }
    
    }

    bool isMoving() const {
        return moving;
    }

    sf::Vector2f const& getVelocity() const {
        return velo;
    }

    PlayerSprites getSprites() {
        return { &bodySprite, &handsSprite, &feetSprite};
    }
    
    std::string const& getDisplayName() const {
        return displayName;
    }


    void setDisplayName(std::string const& name) {
        displayName = name;
        displayNameText.setString(displayName);
    }
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();

        target.draw(feetSprite, states);
        target.draw(bodySprite, states);
        target.draw(handsSprite, states);
        target.draw(displayNameText, states);
    }
private:
    sf::Sprite bodySprite;
    sf::Sprite handsSprite;
    sf::Sprite feetSprite;
    sf::Vector2f velo;
    bool inv;
    bool moving;
    bool controlled;
    std::string displayName;

    PlayerAnimation anim;
    AttackState state;
    sf::Clock attackTimer;

private:
    sf::Font displayNameFont;
    sf::Text displayNameText;
};

