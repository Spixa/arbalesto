#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class AnimatedSprite : public sf::Drawable, public sf::Transformable {
public:
    AnimatedSprite(std::string const& prefix, sf::Vector2u dims, float switch_time);
    virtual ~AnimatedSprite();

    void update(sf::Time dt, int row, bool invert);
    void setWalking(bool walking);
    sf::FloatRect getGlobalBounds(sf::Transform const& upper) {
        sf::Transform combined = upper * getTransform() * sprite.getTransform();

        return combined.transformRect(sprite.getLocalBounds());
    }
private:
    sf::RectangleShape sprite;
    sf::Vector2u image_count;
    sf::Vector2u current_image;
    sf::IntRect anim_rect;
    bool invert;
    float switch_time;
    float total_time;

    std::shared_ptr<sf::Texture> idle_texture;
    std::shared_ptr<sf::Texture> walk_texture;
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};