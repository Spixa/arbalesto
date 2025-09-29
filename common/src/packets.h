#include <cinttypes>
#include <SFML/Network.hpp>

enum class PacketType : uint8_t {
    JoinRequest,
    JoinAccept,
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

    sf::Vector2f pos;
    sf::Vector2f vel;
    std::string uname;
};

inline sf::Packet& operator <<(sf::Packet& p, PlayerState const& s) {
    return p << s.id << s.pos.x << s.pos.y << s.vel.x << s.vel.y;
}
inline sf::Packet& operator >>(sf::Packet& p, PlayerState& s) {
    return p >> s.id >> s.pos.x >> s.pos.y >> s.vel.x >> s.vel.y;
}