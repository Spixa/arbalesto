#include "packets.h"

void NetChunk::serialize(sf::Packet& packet) const {
    packet << x << y;
    for (auto tile : tiles) packet << static_cast<uint32_t>(tile);
}