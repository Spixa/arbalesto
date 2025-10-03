#include "chatbox.h"

#include "../game.h"

// thanks GPT
sf::Color color_from_code(char code) {
    switch (code) {
        case '0': return sf::Color(0, 0, 0);           // Black
        case '1': return sf::Color(0, 0, 170);         // Dark Blue
        case '2': return sf::Color(0, 170, 0);         // Dark Green
        case '3': return sf::Color(0, 170, 170);       // Dark Aqua
        case '4': return sf::Color(170, 0, 0);         // Dark Red
        case '5': return sf::Color(170, 0, 170);       // Dark Purple
        case '6': return sf::Color(255, 170, 0);       // Gold
        case '7': return sf::Color(170, 170, 170);     // Gray
        case '8': return sf::Color(85, 85, 85);        // Dark Gray
        case '9': return sf::Color(85, 85, 255);       // Blue
        case 'a': case 'A': return sf::Color(85, 255, 85);   // Green
        case 'b': case 'B': return sf::Color(85, 255, 255);  // Aqua
        case 'c': case 'C': return sf::Color(255, 85, 85);   // Red
        case 'd': case 'D': return sf::Color(255, 85, 255);  // Light Purple
        case 'e': case 'E': return sf::Color(255, 255, 85);  // Yellow
        case 'f': case 'F': return sf::Color(255, 255, 255); // White
        default: return sf::Color::White;             // fallback
    }
}

ChatBox::ChatBox(sf::Font const& font) : font(font) {
    bottom_y = Game::getInstance()->getUIBounds().size.y - 50.f;
}

ChatLine parse_from_str(sf::String const& raw, const sf::Font& font, unsigned int char_size) {
    ChatLine line;

    sf::Color current_color = sf::Color::White;
    sf::String current_str;

    auto push_seg = [&](sf::String const& text) {
        if (text.isEmpty()) return;
        sf::Text txt(font, text, char_size);
        txt.setFillColor(current_color);
        float w = txt.getLocalBounds().size.x;
        line.segs.push_back({txt, w});
    };

    for (size_t i = 0; i < raw.getSize(); ++i) {
        if (raw[i] == '&' && i + 1 < raw.getSize()) {
            push_seg(current_str);
            current_str.clear();
            current_color = color_from_code(raw[i + 1]);
            ++i;
        } else {
            current_str += raw[i];
        }
    }
    push_seg(current_str);

    if (!line.segs.empty()) {
        line.height = line.segs.front().text.getCharacterSize() + 2.f;
    }

    line.bg.setSize({Game::getInstance()->getUIBounds().size.x / 2.5f, line.height});
    line.bg.setFillColor(sf::Color{30, 30, 30, 60});

    return line;
}


void ChatBox::push(sf::String const& raw) {
    lines.push_back(parse_from_str(raw, font, 23.f));
    if (lines.size() > max_lines) lines.pop_front();

    show_timer = visible_time;
}

void ChatBox::update(sf::Time dt, bool focused) {
    for (auto& line : lines) {
        line.alive_time += dt.asSeconds();
    }

    if (focused) {
        for (auto& line : lines) {
            line.alive_time = 0.f;
        }
    }
}

void ChatBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!always_visible && show_timer <= 0.f) return;

    float y = bottom_y;
    for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
        float x = left_x;

        float alpha = 255.f;
        // thanks GPT
        if (!always_visible) {
            float t = it->alive_time / it->max_alive; // 0..1
            t = std::clamp(t, 0.f, 1.f);

            // cubic ease-out fade
            float fade = 1.f - (t * t * t);
            alpha *= fade;
        }

        {
            auto copy = it->bg;
            copy.setPosition({x, y - it->height});
            sf::Color c = copy.getFillColor();
            c.a = static_cast<uint8_t>(alpha / 2);
            copy.setFillColor(c);
            target.draw(copy, states);
        }
        for (auto seg : it->segs) {
            seg.text.setPosition({x, y - it->height});
            sf::Text copy = seg.text;
            sf::Color c = copy.getFillColor();
            c.a = static_cast<uint8_t>(alpha);
            copy.setFillColor(c);

            target.draw(copy, states);
            x += seg.width;
        }

        y -= it->height;
    }
}

// void ChatBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
//     float y = bottom_y - 20.f;

//     for (auto& line : lines) {
//         float alpha = 255.f;

//         if (!always_visible) {
//             float t = line.alive_time / line.max_alive;
//             alpha = 255.f * (1.f - std::clamp(t, 0.f, 1.f));
//         }

//         for (auto& seg : line.segs) {
//             sf::Text copy = seg.text;
//             sf::Color c = copy.getFillColor();
//             c.a = static_cast<uint8_t>(alpha);
//             copy.setFillColor(c);
//             copy.setPosition(seg.text.getPosition());
//             target.draw(copy, states);
//         }

//         y -= 20.f; // stack upward
//     }
// }
