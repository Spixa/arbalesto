#pragma once

#include <cinttypes>
#include <SFML/Network.hpp>
#include <cstring>

constexpr int TICKRATE = 50;

enum class PacketType : uint8_t {
    JoinRequest,
    JoinAccept,
    Input,
    Snapshot,
    Shoot,
    ChunkRequest,
    ChunkData,
    Teleport,
    ChatMessage,
    WarningMessage,
    TileEdit,
    TileUpdate
};

struct PacketHeader {
    uint32_t seq;
    PacketType type;
};

inline sf::Packet& operator<<(sf::Packet& p, PacketHeader const& h) {
    return p << h.seq << static_cast<uint8_t>(h.type);
}
inline sf::Packet& operator>>(sf::Packet& p, PacketHeader& h) {
    uint8_t t;
    p >> h.seq >> t;
    h.type = static_cast<PacketType>(t);
    return p;
}

constexpr uint8_t CHUNK_SIZE = 16;
constexpr float TILE_SIZE = 32.f;
enum class Tile: uint32_t {
    Grass = 0,
    Water = 1,
    Wood = 2,
    Cobble = 3,
    Sand = 4,
};

struct NetChunk {
    int32_t x, y;
    std::array<Tile, CHUNK_SIZE * CHUNK_SIZE> tiles;
    bool dirty = false;
    void serialize(sf::Packet& packet) const;
};

struct TileEdit {
    int32_t cx, cy;
    uint8_t lx, ly;
    uint32_t dword;
};

struct TileUpdate {
    int32_t cx, cy;
    uint8_t lx, ly;
    uint32_t dword;
};

struct PlayerState {
    uint32_t id;
    std::string uname;
    sf::Vector2f pos;
    sf::Vector2f vel;
    uint16_t held_item = 0;
    float rotation = 0.f;
    std::chrono::steady_clock::time_point last_seen;
};
inline sf::Packet& operator <<(sf::Packet& p, PlayerState const& s) {
    return p << s.id << s.uname << s.pos.x << s.pos.y << s.vel.x << s.vel.y << s.held_item << s.rotation;
}
inline sf::Packet& operator >>(sf::Packet& p, PlayerState& s) {
    return p >> s.id >> s.uname >> s.pos.x >> s.pos.y >> s.vel.x >> s.vel.y >> s.held_item >> s.rotation;
}

inline sf::Packet& operator<<(sf::Packet& pkt, const TileEdit& e) {
    return pkt << e.cx << e.cy << e.lx << e.ly << e.dword;
}

inline sf::Packet& operator>>(sf::Packet& pkt, TileEdit& e) {
    return pkt >> e.cx >> e.cy >> e.lx >> e.ly >> e.dword;
}

inline sf::Packet& operator<<(sf::Packet& pkt, const TileUpdate& u) {
    return pkt << u.cx << u.cy << u.lx << u.ly << u.dword;
}

inline sf::Packet& operator>>(sf::Packet& pkt, TileUpdate& u) {
    return pkt >> u.cx >> u.cy >> u.lx >> u.ly >> u.dword;
}
