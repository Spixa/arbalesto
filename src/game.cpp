#include "game.hpp"

#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

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
    if (!instance_) {
        L("Game::getInstance() called when game wasn't initalized, creating game");
        instance_ = new Game();
    }
    return instance_;
}

Game::Game()
:   window_(sf::VideoMode(1280, 720), "Arbalesto v1", sf::Style::Close),
    general_font_(),
    statistics_text_(),
    state_man_(),
    client()
{
    general_font_.loadFromFile("../res/fonts/main.ttf");
    statistics_text_.setFont(general_font_);
    statistics_text_.setPosition(5.f, 5.f);
    statistics_text_.setCharacterSize(12);
}

void Game::run(std::string const& nickname, std::string const& ip, unsigned short port) {
    sf::Clock clock{};

    client.connect(ip, port);
    client.run();

    sf::Packet packet;
    packet << net::Packet::ClientNickPacket << nickname;
    client.sendPacket(packet);

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
                log("main").info("Goodbye!");
                exit(0);
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Key::P) {
                    sf::Packet packet;
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    packet << net::Packet::PingPacket;
                    client.sendPacket(packet);
                }
                break;
		}
	}
}

void Game::update(sf::Time elapsedTime) {
    updateStatistics(elapsedTime);
    state_man_.update(elapsedTime, &client);
}

void Game::updateStatistics(sf::Time elapsedTime) {
    auto time = std::to_string(elapsedTime.asMilliseconds());
    if (time == "0") {
        statistics_text_.setString("Last update took less than a millisecond!\nFPS: " + std::to_string(int(std::floor(1.f/elapsedTime.asSeconds()))) + "\nTick speed: 20 T/s");
    } else {
        statistics_text_.setString("Last update took " + time + "ms \nFPS: " + std::to_string(int(std::floor(1.f/elapsedTime.asSeconds()))) + "\nTick speed: 20 T/s");
    }    
}

void Game::render() {
    window_.clear();

    state_man_.render(window_);
    window_.draw(statistics_text_);
    window_.display();
}