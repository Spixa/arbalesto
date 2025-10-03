#include "game.h"
#include <iostream>

Game* Game::instance = nullptr;

Game* Game::getInstance() {
    if (!instance) {
        instance = new Game();
    }

    return instance;
}

Game::Game()
    : window(sf::VideoMode{{800, 450}}, "Arbalesto", sf::Style::Close),
    dt_clock(), state_man(), fps(*font_man.get("fallback", "../res/boldpixels.ttf").get()),
    warning(*font_man.get("fallback"), 5.f)
{
    texture_man.get("player_idle", "../res/entity/player/idle.png");
    texture_man.get("player_walk", "../res/entity/player/walk.png");
    texture_man.get("tilesheet", "../res/tilemap/tileset.png");
    texture_man.get("items", "../res/items/items.png");
    texture_man.get("arrow", "../res/entity/arrow/arrow.png");
    texture_man.get("obj_atlas", "../res/objects/atlas.png");

    snd_man.get("flintlock_shoot", "../res/items/flintlock/shoot.ogg");

    fps.setOutlineColor(sf::Color::Black);
    fps.setOutlineThickness(2.f);
    fps.setStyle(sf::Text::Style::Bold);
    fps.setScale({0.5, 0.5});
    def_view = window.getDefaultView();
}

void Game::run() {
    ui_view = window.getDefaultView();
    state_man.init();

    try {
        while (window.isOpen()) {
            sf::Time elapsed = dt_clock.restart();

            proc_events(elapsed);
            update(elapsed);
            render();

            if (first_shot) {
                first_shot = false;
            }
        }
    } catch(std::runtime_error const& e) {
        std::cerr << "ran into an error: " << e.what() << std::endl;
    }
}

void Game::proc_events(sf::Time elapsed) {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            exit(0);
        }

        state_man.update_event(event);
    }
}

void Game::update(sf::Time elapsed) {
    ui_view.setSize({float(window.getSize().x), float(window.getSize().y)});
    ui_view.setCenter({window.getSize().x / 2.f, window.getSize().y / 2.f});
    warning.update(elapsed);
    update_stats(elapsed);
    state_man.update(elapsed);
}

void Game::update_stats(sf::Time elapsed) {
    if (fps_clock.getElapsedTime().asMilliseconds() >= 250) {
        std::string fps_str = std::to_string(int(1.f / elapsed.asSeconds()));
        fps.setString("Arbalesto alpha v0.1.9 (fps: " + fps_str + ") " + getWorld()->getTimeOfDay() + "\n" + etc_info);
        fps_clock.restart();
    }
}

void Game::render() {
    // window.clear({34, 42, 111});
    window.clear();

    window.setView(state_man.getCurrentView());
    state_man.render(window);

    window.setView(ui_view);
    window.draw(fps);
    window.draw(warning);
    state_man.render_gui(window);

    window.display();
}

void Game::tell(sf::String const& raw) {
    state_man.getGameState()->tell(raw);
}