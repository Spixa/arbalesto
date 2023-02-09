#pragma once

#include <inttypes.h>

// Player specification for the Arbalesto Server
// Player instances and their info are stored like this in the ram
// class Player in namespace "server" should not be confused with class Player

namespace server {
    struct PlayerAnimation {
        enum class FeetAnimation: uint8_t {
            RunningAnimation = 0,
            WalkingAnimation = 1,
            IdleAnimation = 2
        };
        enum class BodyAnimation: uint8_t {

        };
    };

    struct Player {
        float xPos, yPos;
        PlayerAnimation animation;
    };
}