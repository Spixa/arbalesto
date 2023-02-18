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
            processEvents(elapsedTime);
            update(elapsedTime);
            render();
        }
    } catch (std::runtime_error const& e) {
        std::cout << e.what() << std::endl;
    }
}

#ifdef __unix__
#include <iostream>
#include <fstream>
#include <unistd.h>

void process_mem_usage(double& vm_usage, double& resident_set)
{
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}
#endif

void Game::processEvents(sf::Time elapsedTime)
{
	sf::Event event;
	while (window_.pollEvent(event))
	{
		switch (event.type)
		{
            case sf::Event::Closed:
                log("main").info("Goodbye!");
                log("launcher").info("--- <Summary> ---");
                log("launcher").info("Final FPS: " + std::to_string(int(std::floor(1.f/elapsedTime.asSeconds()))));
                log("launcher").info("Discarded packets: " + std::to_string(failedPacketCounter));
                #ifdef __unix__
                    double vm, rss;
                    process_mem_usage(vm, rss);
                    log("launcher").info("<Memory> (Unix-only)");
                    log("launcher").info("\tVM: " + std::to_string(vm) + " RSS: " + std::to_string(rss));
                    log("launcher").info("</Memory>");
                #endif
                log("launcher").info("--- </Summary> ---");
                window_.close();
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
            case sf::Event::LostFocus:
                focused = false;
                break;
            case sf::Event::GainedFocus:
                focused = true;
                break;
		}
	}
}

void Game::update(sf::Time elapsedTime) {
    updateStatistics(elapsedTime);
    state_man_.update(elapsedTime, &client);
}

bool Game::isFocused() {
    return focused;
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