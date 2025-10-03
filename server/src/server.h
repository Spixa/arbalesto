#pragma once

#include "packets.h"
#include <unordered_map>
#include <cmath>

constexpr uint8_t CHUNK_SIZE = 16;
constexpr float TILE_SIZE = 32.f;

struct Remote {
    sf::IpAddress addr;
    unsigned short port;
    PlayerState state;

    Remote(sf::IpAddress a, unsigned short p, PlayerState s)
        : addr(a), port(p), state(std::move(s)) {}
    Remote(Remote&&) noexcept = default;
    Remote& operator=(Remote&&) noexcept = default;
    Remote() = delete;
};


inline sf::Vector2i worldToChunkCoords(sf::Vector2f pos) {
    int cx = static_cast<int>(std::floor(pos.x / (CHUNK_SIZE * TILE_SIZE)));
    int cy = static_cast<int>(std::floor(pos.y / (CHUNK_SIZE * TILE_SIZE)));
    return {cx, cy};
}

class Server {
    sf::UdpSocket sock;
    uint32_t next_seq{1};
    uint32_t next_id{1};
    std::unordered_map<uint32_t, PlayerState> players;
    std::unordered_map<uint32_t, Remote> remotes;
public:
    ~Server() { sock.unbind(); }
    void bind(unsigned short port);
    void run();

    std::optional<uint32_t> findRemoteIdByEndpoint(const sf::IpAddress& a, unsigned short port) const;
private:
    void sync(Remote& remote);
    void update();
    void recv();
private:
    const float TICK_SECS = 1.f / TICKRATE;
};
