#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class AnimatedTexture {
public:
    AnimatedTexture(std::string const& location, sf::Vector2u imageCount, float switchTime) : imageCount(imageCount), switchTime(switchTime) {
        texture.loadFromFile(location);
        totalTime = 0.0f;
    
        animRect.width = texture.getSize().x / float(imageCount.x);
	    animRect.height = texture.getSize().y / float(imageCount.y);
    }

    void update(int row, float deltaTime) {
        currentImage.y = row;
        totalTime += deltaTime;
        if (totalTime >= switchTime) {
            totalTime -= switchTime;
            currentImage.x++;

            if (currentImage.x >= imageCount.x) {
                currentImage.x = 0;

            }
        }
        animRect.left = currentImage.x * animRect.width;
        animRect.top = currentImage.y * animRect.height;
    }
    
    sf::Texture& getTexture() { return texture; }
    sf::Vector2u getImageCount() const { return imageCount; }
    sf::Vector2u getCurrentImage() const { return currentImage; }
    sf::IntRect getAnimRect() { return animRect; }

    void setCurrentImage(sf::Vector2u const& vec) { currentImage = vec; }
    void setCurrentImageX(unsigned int x) { currentImage.x = x; }
    void setCurrentImageY(unsigned int y) { currentImage.y = y; }   
private:
    sf::Texture texture{};
    sf::Vector2u imageCount;
    sf::Vector2u currentImage;
    sf::IntRect animRect;

    float totalTime, switchTime;
};

class Player;
enum class AttackState;
class PlayerAnimation {
public:
    PlayerAnimation(float switchTime, Player* player);
    void update(int row, sf::Time deltaTime, bool i, AttackState state);

    void setFromPacket(std::string const& animPacket);
    std::string exportToPacket();
private:
    AnimatedTexture body, hand, feet;
    std::string handText, bodyText, feetText;
    Player* player;
};