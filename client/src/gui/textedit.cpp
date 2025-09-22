#include "textedit.h"

TextEdit::TextEdit(sf::String const& prefix, sf::Font const& font, unsigned int char_size) : prefix(prefix), content(""), text(font), cb(nullptr) {
    text.setCharacterSize(char_size);
    text.setFillColor(sf::Color::White);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(2.f);

    background.setFillColor(sf::Color(50, 50, 50, 150));
    update_text();
}

void TextEdit::input(char c) {
    if (!focused) return;

    constexpr char BACKSPACE = 8;
    constexpr char ENTER1 = 13; // '\r'
    constexpr char ENTER2 = 10; // '\n'
    constexpr char ESCAPE = 27;

    if (c == BACKSPACE) {
        if (!content.isEmpty())
            content.erase(content.getSize() - 1, 1);
    } else if (c == ENTER1 || c == ENTER2) {
        if (cb) cb(content);
        content.clear();
        focused = false;
    } else if (c == ESCAPE) {
        content.clear();
        focused = false;
    } else if (c >= 32 && c < 127) /* printable */ {
        content += c;
    }

    update_text();
}

void TextEdit::update() {
    if (blink.getElapsedTime() >= sf::seconds(0.5)) {
        cursor = !cursor;
        blink.restart();
        update_text();
    }
}

void TextEdit::update_text() {
    if (cursor)
        text.setString(prefix + content + "_");
    else
        text.setString(prefix + content);
}

void TextEdit::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!focused) return;

    states.transform *= getTransform();
    target.draw(background, states);
    target.draw(text, states);
}