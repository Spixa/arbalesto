#pragma once
#include "util.h"
#include <SFML/Network.hpp>

class ClientNetwork : NonCopyable {
public:
    void join(sf::IpAddress const& ip, sf::String const& username);
    static ClientNetwork* getInstance();
private:
    ClientNetwork();
    ~ClientNetwork();
private:
    static ClientNetwork* instance;
private:
    sf::TcpSocket socket;
};