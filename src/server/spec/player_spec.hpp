#pragma once

#include <inttypes.h>
#include <string>

// Player specification for the Arbalesto Server
// Player instances and their info are stored like this in the ram
// class Player in namespace "server" should not be confused with class Player

namespace server {
    struct PlayerAnimation {
        enum class MovementState: uint8_t {
            RunningAnimation = 0,
            WalkingAnimation = 1,
            IdleAnimation = 2
        } movement = MovementState::IdleAnimation;
        enum class AttackState: uint8_t {
            Fist = 0,
            SwordAttack = 2,
            SwordHold = 1,
        } attack = AttackState::Fist;
    };

    struct Player {
    public:
        Player(std::string const& displayName, sf::Vector2f startingPos) 
            : displayName(displayName), xPos(startingPos.x), yPos(startingPos.y),
              animation()
        {

        }
    public:
        float xPos, yPos;
        std::string displayName;
        PlayerAnimation animation;
    };
}