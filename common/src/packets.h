#include <cinttypes>
#include <SFML/Network.hpp>

constexpr int TICKRATE = 50;

enum class PacketType : uint8_t {
    JoinRequest,
    JoinAccept,
    Input,
    Snapshot,
    Shoot,
    Place,
    Break,
    ChunkData,
    ChatMessage,
    WarningMessage
};

struct PacketHeader {
    uint16_t seq;
    PacketType type;
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