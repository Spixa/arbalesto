#pragma once
#include <SFML/Network.hpp>
#include <memory>

class Client {
private:
    sf::TcpSocket* sock;
    std::string name;
public:
    Client(sf::TcpSocket* sock) : sock(sock) {

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