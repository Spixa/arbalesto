#pragma once
#include <SFML/Network.hpp>
#include <memory>

class Client {
private:
    sf::TcpSocket* sock;
public:
    Client(sf::TcpSocket* sock) : sock(sock) {

    }

    ~Client() { delete sock; }

    sf::TcpSocket* socket() {
        return sock;
    }
};