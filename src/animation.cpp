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

    L("Applied BODY texture layer to animation of Player");
    player->getSprites().body->setTexture(body.getTexture());
    L("Applied HAND texture layer to animation of Player");
    player->getSprites().hand->setTexture(hand.getTexture());
    L("Applied FEET texture layer to animation of Player");
    player->getSprites().feet->setTexture(feet.getTexture());
}

void lex(std::string str, std::string arg[], char seprator) {
    std::string word = "";
    int i = 0;
    for (auto x : str) {
        if (x == seprator)
        {
            i++;
            arg[i] = word;
            word = "";
        }
        else
        {
            word = word += x;
        }
        arg[i] = word;
    }
}

void PlayerAnimation::setFromPacket(std::string const& animPacket) {
    // set animation from packet
    std::string parts[3];
    lex(animPacket, parts, ':');

    std::string& feet = parts[0];
    std::string& body = parts[1];
    std::string& head = parts[2];
}

std::string PlayerAnimation::exportToPacket() {

    
}

void PlayerAnimation::update(int row, sf::Time deltaTime, bool i, AttackState state )
{
    float dt = deltaTime.asSeconds();
    
    switch (state) {
        case AttackState::Fist: {
            body.update(row, dt);
            hand.update(row, dt);
        } break;
        case AttackState::SwordHold: {
            body.update(row, dt);
            
            if (row == 0)
                hand.update(4, dt);
            else
                hand.update(2, dt);
        } break;
        case AttackState::SwordAttack: {
            body.update(row, dt);
            hand.update(3, dt);
        }
    }

    feet.update(row, dt);

    auto b = body.getAnimRect();
    auto h = hand.getAnimRect();
    auto f = feet.getAnimRect();

    player->getSprites().body->setTextureRect(b);
    player->getSprites().hand->setTextureRect(h);
    player->getSprites().feet->setTextureRect(f);
}
