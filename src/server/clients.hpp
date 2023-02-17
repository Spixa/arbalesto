#pragma once
#include <SFML/Network.hpp>
#include <memory>

class Client {
private:
    sf::TcpSocket* sock;
    std::string name;

    sf::Vector2f position;
public:
    Client(sf::TcpSocket* sock) : sock(sock) {

    }

    sf::Vector2f const& getPosition() const {
        return position;
    }

    void setPosition(sf::Vector2f const& pos) {
        position = pos;
    }

    ~Client() { delete sock; }

    sf::TcpSocket* socket() {
        return sock;
    }

    void setName(const std::string& name) {
        this->name = name;
    }

    std::string getName() const {
        return name;
    }
};