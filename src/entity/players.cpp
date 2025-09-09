#include "players.h"
#include "../game.h"
#include "arrow.h"
#include "../item/sword.h"
#include "../item/bow.h"

#include <cmath>

Player::Player(ItemType holding_stuff, sf::Vector2f spawn, float health) : Entity(next(), EntityType::PlayerEntity, health), sprite("player", {4,5}, 0.1) {
    if (holding_stuff == ItemType::Bow) holding = std::make_unique<Bow>();
    else holding = std::make_unique<Sword>();
    setPosition(spawn);

    health_box.setSize({16, 2});
    health_bar.setSize({16, 2});

    health_box.setFillColor(sf::Color(169, 169, 169));
    health_bar.setFillColor(sf::Color::Red);

    health_box.setOutlineColor(sf::Color::Black);
    health_box.setOutlineThickness(.4f);

    health_bar.setPosition({-8, -12});
    health_box.setPosition({-8, -12});
}

Player::~Player() {}

void Player::update(sf::Time elapsed) {
    update_derived(elapsed);

    Facing f = getFacing();
    if (f != Facing::None) {
        facing = f;
    }
    switch (facing) {
        case Facing::North: {
            row = 4;
        }
        break;
        case Facing::South: {
            row = 0;
        } break;
        case Facing::East: {
            row = 2;
            inv = true;
        } break;
        case Facing::West: {
            row = 2;
            inv = false;
        } break;
        case Facing::Southeast: {
            row = 1;
            inv = true;
        } break;
        case Facing::Southwest: {
            row = 1;
            inv = false;
        } break;
        case Facing::Northeast: {
            row = 3;
            inv = true;
        } break;
        case Facing::Northwest: {
            row = 3;
            inv = false;
        } break;
    }

    if (velocity.length() == 0) {
        sprite.setWalking(false);
    } else {
        sprite.setWalking(true);
    }

    sprite.update(elapsed, row, inv);

    // MOVING LOGIC - sliding collisions
    sf::Vector2f currentPos = getPosition();
    sf::Vector2f proposedMove = velocity * elapsed.asSeconds();
    sf::Vector2f halfSize{8.f, 8.f}; // player is 16x16px

    // x movement
    if (proposedMove.x != 0.f) {
        float edgeX = currentPos.x + proposedMove.x + (proposedMove.x > 0 ? halfSize.x : -halfSize.x);
        bool blockedX = false;

        for (float y = currentPos.y - halfSize.y; y <= currentPos.y + halfSize.y; y += 4.f) {
            if (Game::getInstance()->getWorld()->isSolidAt({edgeX, y}, {1.f, 1.f})) {
                blockedX = true;
                break;
            }
        }

        if (!blockedX)
            currentPos.x += proposedMove.x;
        else
            velocity.x = 0.f;
    }

    if (proposedMove.y != 0.f) {
        float edgeY = currentPos.y + proposedMove.y + (proposedMove.y > 0 ? halfSize.y : -halfSize.y);
        bool blockedY = false;

        for (float x = currentPos.x - halfSize.x; x <= currentPos.x + halfSize.x; x += 4.f) {
            if (Game::getInstance()->getWorld()->isSolidAt({x, edgeY}, {1.f, 1.f})) {
                blockedY = true;
                break;
            }
        }

        if (!blockedY)
            currentPos.y += proposedMove.y;
        else
            velocity.y = 0.f;
    }

    setPosition(currentPos);
}

void Player::update_tick(sf::Time elapsed) {
    float ratio = health / initial_health;

    health_bar.setSize({ratio * 16.f, 2});

    sf::Color color(sf::Color::Magenta);
    if (!invincible)
        color = sf::Color(
            static_cast<uint8_t>((1.f - ratio) * 255), // Red increases as health decreases
            static_cast<uint8_t>(ratio * 255),         // Green decreases as health decreases
            0                                            // Blue stays 0
        );

    health_bar.setFillColor(color);
    update_tick_derived(elapsed);
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();

    if (holding) {
        if (facing == Facing::North || facing == Facing::Northeast || facing == Facing::Northeast) {
            target.draw(*holding, states);
            target.draw(sprite, states);
        } else {
            target.draw(sprite, states);
            target.draw(*holding, states);
        }

    } else {
        target.draw(sprite, states);
    }
    target.draw(health_box, states);
    target.draw(health_bar, states);
}

ControllingPlayer::~ControllingPlayer() {
    // Game::getInstance()->getWorld()->addEntity(std::make_unique<ControllingPlayer>());
}

void ControllingPlayer::update_derived(sf::Time elapsed) {

    if (Game::getInstance()->getWindow().hasFocus())
        velocity = sf::Vector2f{
            float(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)),
            float(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        };

    constexpr float speed = 150.0;
    float dt = elapsed.asSeconds();

    if (velocity != sf::Vector2f()) {
        velocity = velocity.normalized();
        velocity.x = velocity.x * speed;
        velocity.y = velocity.y * speed;
    }

    if (holding) {
        sf::Vector2f player_pos = getPosition() + sf::Vector2f{0, 0};
        sf::Vector2f mouse_world = Game::getInstance()->getMouseWorld();
        sf::Vector2f looking_at = mouse_world - player_pos;

        bool facing_left = (mouse_world.x < player_pos.x);

        if (holding->getType() == ItemType::Bow) {
            sf::Vector2f la;
            if (looking_at.length() != 0.f)
                la = looking_at.normalized();
            sf::Angle rotation = sf::radians(std::atan2(la.y, la.x)) + sf::degrees(-135);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && arrow_cooldown.getElapsedTime() >= sf::seconds(0.125)) {
                arrow_cooldown.restart();
                auto arrow_ptr = std::make_unique<Arrow>(player_pos, looking_at, 300.f, sf::seconds(5.f), getId(), velocity);
                // do stuff on the arrow here
                // end of stuff done on arrow
                Game::getInstance()->getWorld()->addEntity(std::unique_ptr<Entity>(std::move(arrow_ptr)));
            }

            holding->setRotation(rotation);
            holding->update(elapsed, false); // bow is full normal rotation, no horizantal
        } else {
            holding->update(elapsed, !facing_left); // other items rotate only rotate horizantally
        }
    }
}

void AiPlayer::update_tick_derived(sf::Time dt) {
    if (decisionClock.getElapsedTime() >= decisionInterval) {
        decisionClock.restart();

        // 20% chance to idle
        if (rand() % 5 == 0) {
            velocity = {0.f, 0.f};
        } else {
            int dir = rand() % 8;

            switch (dir) {
                case 0: velocity = {0.f, -1.f}; break;       // up
                case 1: velocity = {1.f, -1.f}; break;       // up-right
                case 2: velocity = {1.f, 0.f}; break;        // right
                case 3: velocity = {1.f, 1.f}; break;        // down-right
                case 4: velocity = {0.f, 1.f}; break;        // down
                case 5: velocity = {-1.f, 1.f}; break;       // down-left
                case 6: velocity = {-1.f, 0.f}; break;       // left
                case 7: velocity = {-1.f, -1.f}; break;      // up-left
            }

            if (velocity.x != 0 && velocity.y != 0)
                velocity /= std::sqrt(2.f);
        }

        float nextInterval = 1.f + static_cast<float>(rand() % 2000) / 1000.f;
        decisionInterval = sf::seconds(nextInterval);

        constexpr float speed = 50.f;
        velocity *= speed;
    }

    if (!currentTarget || !currentTarget->isAlive()) {
        currentTarget = Game::getInstance()->getWorld()->getNearestEntity(this);
    }

    if (currentTarget && holding && holding->getType() == ItemType::Bow) {
        sf::Vector2f my_pos = getPosition();
        sf::Vector2f target_pos = currentTarget->getPosition();
        sf::Vector2f desiredDir = target_pos - my_pos;

        if (desiredDir.x != 0.f || desiredDir.y != 0.f)
            desiredDir /= std::sqrt(desiredDir.x*desiredDir.x + desiredDir.y*desiredDir.y);

        // SMOOTH ROTATION IMPL
        float rotationSpeed = 5.f; // radians per second
        sf::Vector2f currentDir = holdingDirection; // AI stores current aiming direction
        float dot = currentDir.x*desiredDir.x + currentDir.y*desiredDir.y;
        dot = std::clamp(dot, -1.f, 1.f);
        float angleDiff = std::acos(dot);

        // determine rotation direction using cross product
        float cross = currentDir.x*desiredDir.y - currentDir.y*desiredDir.x;
        if (angleDiff > 0.001f) {
            float maxRotate = rotationSpeed * dt.asSeconds();
            float rotateAmount = std::min(maxRotate, angleDiff);
            if (cross < 0) rotateAmount = -rotateAmount;

            float cosA = std::cos(rotateAmount);
            float sinA = std::sin(rotateAmount);
            sf::Vector2f newDir = {
                currentDir.x * cosA - currentDir.y * sinA,
                currentDir.x * sinA + currentDir.y * cosA
            };
            holdingDirection = newDir;
        }

        // rotate to match the aiming direction
        sf::Angle rotation = sf::radians(std::atan2(holdingDirection.y, holdingDirection.x)) + sf::degrees(-135);
        holding->setRotation(rotation);
        holding->update(dt, false); // no inversion as this is not a melee weapon

        // shooting logic
        if (arrowCooldown.getElapsedTime() >= arrowInterval) {
            arrowCooldown.restart();
            float arrowSpeed = 300.f;
            sf::Time arrowLifetime = sf::seconds(5.f);
            auto arrow_ptr = std::make_unique<Arrow>(my_pos, holdingDirection, arrowSpeed, arrowLifetime, getId());
            Game::getInstance()->getWorld()->addEntity(std::move(arrow_ptr));

            float nextInterval = 0.006f + static_cast<float>(rand() % 500) / 1000.f; // between 0 and 2 seconds
            arrowInterval = sf::seconds(nextInterval);
        }
    }
}
