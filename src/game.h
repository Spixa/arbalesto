#pragma once

#include "util.h"
#include "res_man.h"
#include "focus.h"
#include "state/state_man.h"
#include "state/game_state.h"
#include <SFML/Graphics.hpp>

class World;
class Game : NonCopyable {
public:
    void run();
    static Game* getInstance();

    sf::RenderWindow& getWindow() { return window; }
    ResourceManager<sf::Texture>& getTextureManager() { return texture_man; }
    StateManager& getStateManager() { return state_man; }
    World* getWorld() { return state_man.getGameState()->getWorld(); }
    sf::Vector2f getMouseWorld() {
        sf::Vector2i mouse_screen = sf::Mouse::getPosition(Game::getInstance()->getWindow());
        return window.mapPixelToCoords(mouse_screen, state_man.getCurrentView());
    }
    sf::Font& getFallbackFont() {
        return *font_man.get("fallback");
    }
    sf::FloatRect getUIBounds() {
        return {
            {
                ui_view.getCenter().x - ui_view.getSize().x / 2.f,
                ui_view.getCenter().y - ui_view.getSize().y / 2.f
            },
            {
                ui_view.getSize().x,
                ui_view.getSize().y
            }
        };
    }

    bool shouldWorldFocus() {
        return focus_stk.shouldWorldFocus() && window.hasFocus();
    }

    void pushFocus(UIWidget id) { focus_stk.push(id); }
    void popFocus(UIWidget id) { focus_stk.pop(id); }
    std::optional<UIWidget> topmostFocus() const { return focus_stk.topmost(); }

    bool isInitial() const { return first_shot; };

    void setInfo(sf::String const& info) { etc_info = info; }
private:
    Game();
    ~Game();
private:
    void proc_events(sf::Time elapsed);
    void update(sf::Time elapsed);
    void render();
private: // updates
    void update_stats(sf::Time elapsed);
private:
    // internal
    static Game* instance;
    StateManager state_man;
    ResourceManager<sf::Texture> texture_man;
    ResourceManager<sf::Font> font_man;
    sf::RenderWindow window;
    sf::View ui_view;
    sf::Clock dt_clock;
private:
    // app stats
    sf::Text fps;
    sf::String etc_info;
    sf::Clock fps_clock;
private:
    sf::Image icon;
    bool first_shot{true};
    FocusStack focus_stk;
};