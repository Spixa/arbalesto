#include "game.hpp"

#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include "player.hpp"

template <typename T>
std::string toString(const T& value)
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

void log_error(int ec, std::string const& content) {
    throw std::runtime_error("Error code: " + std::to_string(ec) + " " + content);
}

Game* Game::instance_ = nullptr;


Game* Game::getInstance() {
    if (!instance_)
        instance_ = new Game();
    return instance_;
}

Game::Game()
:   window_(sf::VideoMode(1280, 720), "SFML Application", sf::Style::Close),
    general_font_(),
    statistics_text_()
{
    general_font_.loadFromFile("../res/fonts/main.ttf");
    statistics_text_.setFont(general_font_);
    statistics_text_.setPosition(5.f, 5.f);
    statistics_text_.setCharacterSize(12);

    p = new Player();
    boxes.push_back(new Box());
}

void Game::run() {
    sf::Clock clock{};

    try {
        while (window_.isOpen()) {
            sf::Time elapsedTime = clock.restart();

            processEvents();
            update(elapsedTime);
            render();
        }
    } catch (std::runtime_error const& e) {
        std::cout << e.what() << std::endl;
    }
}

void Game::processEvents()
{
	sf::Event event;
	while (window_.pollEvent(event))
	{
		switch (event.type)
		{
            case sf::Event::Closed:
                window_.close();
                break;
		}
	}
}

void Game::update(sf::Time elapsedTime) {
    updateStatistics(elapsedTime);
    p->update(elapsedTime);
}

void Game::updateStatistics(sf::Time elapsedTime) {
    auto time = std::to_string(elapsedTime.asMilliseconds());
    if (time == "0") {
        statistics_text_.setString("Arbalesto: Last update took less than a millisecond!");
    } else {
        statistics_text_.setString("Last update took " + time + "ms");
    }    
}

void Game::render() {
    window_.clear(sf::Color(200, 200, 200));
    window_.draw(statistics_text_);

    for (auto x: boxes) {
        window_.draw(*x);
    }

    window_.draw(*p);

    window_.display();
}