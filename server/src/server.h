#pragma once

#include "packets.h"
#include "generator.h"
#include <unordered_map>
#include <cmath>

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

inline sf::Vector2f tileToWorldCoords(const sf::Vector2i& tile) {
    return sf::Vector2f{
        static_cast<float>(tile.x * TILE_SIZE),
        static_cast<float>(tile.y * TILE_SIZE)
    };
}

struct PairHash {
    std::size_t operator()(const std::pair<int32_t, int32_t>& p) const noexcept {
        // Use 64-bit mixing for good distribution
        // (based on boost::hash_combine)
        std::size_t h1 = std::hash<int32_t>{}(p.first);
        std::size_t h2 = std::hash<int32_t>{}(p.second);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

class Server {
    sf::UdpSocket sock;
    uint64_t tod{0};
    uint32_t next_seq{1};
    uint32_t next_id{1};
    std::unordered_map<uint32_t, PlayerState> players;
    std::unordered_map<uint32_t, Remote> remotes;
    ChunkGenerator generator{1337}; // global or member
public:
    ~Server() { sock.unbind(); }
    void bind(unsigned short port);
    void run();

    std::optional<uint32_t> findRemoteIdByEndpoint(const sf::IpAddress& a, unsigned short port) const;
    void broadcast(sf::String const& raw);
    void tellraw(uint32_t id, sf::String const& raw);
    void teleport(uint32_t id, sf::Vector2f const& to);
public:
    NetChunk getNetChunk(int32_t cx, int32_t cy);
    void saveNetChunk(NetChunk const& nc);
private:
    void sync(Remote& remote);
    void update();
    void recv();
private:
    const float TICK_SECS = 1.f / TICKRATE;
    std::unordered_map<std::pair<int32_t,int32_t>, NetChunk, PairHash> netchunk_cache;
};
