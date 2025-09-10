#pragma once

#include <SFML/Graphics.hpp>
#include <deque>

struct ChatSegment {
    sf::Text text;
    float width; // cached
};

struct ChatLine {
    std::vector<ChatSegment> segs;
    float height; // pre proc'd
    float alive_time = 0.f;
    static constexpr float max_alive = 10.f;
};

ChatLine parse_from_str(sf::String const& raw, sf::Font const& font, unsigned int size);

class ChatBox : public sf::Drawable {
public:
    ChatBox(sf::Font const& font);
    void push(sf::String const& raw);
    void update(sf::Time dt, bool focused);

    void setAlwaysVisible(bool tof) { always_visible = tof; }
protected:
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
private:
    const sf::Font& font;
    std::deque<ChatLine> lines;
    float visible_time = 5.f, show_timer = 0.0;
    bool always_visible = false;

    float bottom_y;
    float left_x = 12.f;
    static constexpr size_t max_lines = 26;
};