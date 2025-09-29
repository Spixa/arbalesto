#include "server.h"
#include <spdlog/spdlog.h>

using namespace std::chrono;
void Server::bind(unsigned short port) {
    if (sock.bind(port) != sf::Socket::Status::Done) {
        spdlog::get("server")->info("Failed to bind to port {}", port);
    }

    sock.setBlocking(false);
    spdlog::get("server")->info("Bound to port {} at 20Hz tickrate", port);
}

void Server::run() {
    const int TPS = 20;
    const auto tick_len = milliseconds(1000 / TPS);

    while (true) {
        auto start = steady_clock::now();
        update();
        auto elapsed = steady_clock::now() - start;

        if (elapsed < tick_len) std::this_thread::sleep_for(tick_len - elapsed);
    }
}

void Server::sync(Remote& remote) {
    sf::Packet pkt;

    pkt << next_seq << (uint8_t) PacketType::Snapshot;

    auto center_chunk = worldToChunkCoords(remote.pos);
    int radius = 1; // 3x3 chunk

    auto in_range = [&](sf::Vector2f pos) {
        sf::Vector2i c = worldToChunkCoords(pos);
        return (std::abs(c.x - center_chunk.x) <= radius &&
                std::abs(c.y - center_chunk.y) <= radius);
    };

    std::vector<PlayerState> nearby;
    for (auto const& [id, state] : players) {
        if (in_range(state.pos)) nearby.push_back(state);
    }

    pkt << (uint32_t) nearby.size();
    for (auto const& pl : nearby) pkt << pl;

    auto status = sock.send(pkt, remote.addr, remote.port);
}

void Server::update() {
    for (auto& [id, remote] : remotes) {
        sync(remote);
    }
    next_seq++;
}
