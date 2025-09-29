#pragma once
#include "util.h"
#include <SFML/Network.hpp>
#include "packets.h"

struct RemotePlayer {
    PlayerState state;
    sf::Vector2f predicted_pos;
};

class ClientNetwork : NonCopyable {
public:
    void join(sf::IpAddress const& ip, unsigned short const& port, sf::String const& username);
    void update(sf::Time elapsed);
    void syncInput(sf::Vector2f const& vel);
    const std::unordered_map<uint32_t, RemotePlayer>& getPlayers() const;
    static ClientNetwork* getInstance();
private:
    ClientNetwork();
    ~ClientNetwork();
private:
    static ClientNetwork* instance;
private:
    sf::TcpSocket socket;
};