#pragma once
#include <SFML/Graphics.hpp>
#include "sprite.hpp"

class Box : public HitboxSprite {
public:
    Box() {
        insideLayer.setFillColor(sf::Color::Red);
        outsideLayer.setFillColor(sf::Color::Blue);

        insideLayer.setSize({100, 50});
        setHitbox({0,0, 100, 50});
        outsideLayer.setSize({100, 50});
    }

public:
    virtual void update(sf::Time deltaTime) override {

    }

    void setIsInside(bool is_inside) {
        isInside = is_inside;
    }
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override { 
        states.transform *= getTransform();

        if (isInside) target.draw(insideLayer, states);
        else target.draw(outsideLayer, states);
    }
protected:
    sf::Texture outsideBoxTexture;
    sf::Texture insideBoxTexture;

    sf::RectangleShape insideLayer;
    sf::RectangleShape outsideLayer;

    bool isInside = false;
};

class ControllableBox : public Box {
public:
    void update(sf::Time deltaTime, Box& other) {
        float dt = deltaTime.asSeconds();
        constexpr float speed = 128.f;

        move({
            speed * dt * (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) - sf::Keyboard::isKeyPressed(sf::Keyboard::Left)),
            speed * dt * (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) - sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        });

        if (getGlobalHitbox().intersects(other.getGlobalHitbox())) {
            isInside = true;
        } else {
            isInside = false;
        }
    }
};