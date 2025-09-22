#include "warning.h"
#include "../game.h"


Warning::Warning(sf::Font& font, float dura) : text(font), dura(dura) {
    text.setCharacterSize(28);
    text.setFillColor(sf::Color::White);

    bg.setFillColor(sf::Color{0, 0, 0, 128});
}

void Warning::sendWarn(sf::String const& msg) {
    text.setString(msg);
    timer.restart();
    active = true;
}

void Warning::update(sf::Time dt) {
    if (!active) return;

    float elapsed = timer.getElapsedTime().asSeconds();

    if (elapsed >= dura) {
        active = false;
        return;
    }

    float a = 255.f;
    a *= cubic_ease(elapsed / dura);
    sf::Color tc = text.getFillColor();
    tc.a = static_cast<uint8_t>(a);
    text.setFillColor(tc);

    sf::Color bc = bg.getFillColor();
    bc.a = static_cast<uint8_t>(128.f * (a/255.f));
    bg.setFillColor(bc);

    update_bg();
}

void Warning::update_bg() {
    // get text bounds
    sf::FloatRect tb = text.getLocalBounds();
    float padding = 5.f;

    bg.setSize({tb.size.x + padding * 2, tb.size.y + padding * 2});
    bg.setOrigin(bg.getSize() * 0.5f);

    text.setOrigin({tb.position.x + tb.size.x/2.f, tb.position.y + tb.size.y/2.f});

    // anchor to center of window
    auto pos = Game::getInstance()->getUIBounds().getCenter();
    pos = {pos.x, pos.y * 2.f - 80.f};
    setPosition(pos);
}

void Warning::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    target.draw(bg, states);
    target.draw(text, states);
}