#pragma once

#include <SFML/Graphics.hpp>
#include <sstream>
#include "../logger.hpp"

#define DELETE_KEY 8
#define ENTER_KEY 13
#define ESCAPE_KEY 27

class Textbox {
public:
    Textbox() {

    }

    Textbox(int size, sf::Color color, bool sel) 
        : isSelected(sel)
    {
        textbox.setCharacterSize(size);
        textbox.setColor(color);
    }

private:
    sf::Text textbox;
    std::ostringstream text;
private:
    bool isSelected = false;
    bool hasLimit = false;
    int limit = 31;
private:
    void inputLogic(int charTyped) {
        // No special case mode
        if (charTyped != DELETE_KEY && charTyped != ENTER_KEY && charTyped != ESCAPE_KEY) {
               
        }
    }
};