#pragma once

#include "packets.h"
#include <unordered_map>
#include <cmath>

constexpr uint8_t CHUNK_SIZE = 16;
constexpr float TILE_SIZE = 32.f;

struct Remote {
    sf::IpAddress addr;
    unsigned short port;
    sf::Vector2f pos;
};

inline sf::Vector2i worldToChunkCoords(sf::Vector2f pos) {
    int cx = static_cast<int>(std::floor(pos.x / (CHUNK_SIZE * TILE_SIZE)));
    int cy = static_cast<int>(std::floor(pos.y / (CHUNK_SIZE * TILE_SIZE)));
    return {cx, cy};
}

class Server {
    sf::UdpSocket sock;
    uint16_t next_seq{1};
    uint32_t next_id{1};
    std::unordered_map<uint32_t, PlayerState> players;
    std::unordered_map<uint32_t, Remote> remotes;
public:
    void bind(unsigned short port);
    void run();
private:
    void sync(Remote& remote);
    void update();
    void recv();
private:
    const int TPS = 20;
    const float TICK_SECS = 1000.f / TPS;
};
