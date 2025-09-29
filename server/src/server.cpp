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
    const auto tick_len = milliseconds(1000 / TPS);

    while (true) {
        auto start = steady_clock::now();
        recv();
        update();
        auto elapsed = steady_clock::now() - start;
        if (elapsed < tick_len) std::this_thread::sleep_for(tick_len - elapsed);
    }
}

void Server::sync(Remote& remote) {
    sf::Packet pkt;

    pkt << next_seq << (uint8_t) PacketType::Snapshot;

    auto center_chunk = worldToChunkCoords(remote.pos);
    int radius = 1; // 3x3 chunks

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

void Server::recv() {
    sf::Packet pkt;
    std::optional<sf::IpAddress> sender;
    unsigned short port;
    while (sock.receive(pkt, sender, port) == sf::Socket::Status::Done) {
        if (!sender) continue; // skip iteration
        sf::IpAddress addr = sender.value();

        uint8_t type;
        pkt >> type;

        switch( (PacketType) type) {
            case PacketType::JoinRequest: {
                std::string uname;
                pkt >> uname;

                uint32_t id = next_id++;
                PlayerState state;
                state.id = id;
                state.uname = uname;
                state.pos = {0.f, 0.f};
                state.vel = {0.f, 0.f};
                players[id] = state;

                remotes.emplace(id, Remote{addr, port, state.pos});

                spdlog::get("auth")->info("Player '{}' joined (id={}) from {}:{}", uname, id, addr.toString(), port);

                sf::Packet reply;
                reply << (uint8_t) PacketType::JoinAccept << id << (uint32_t) players.size();

                for (auto const& [pid, p] : players) {
                    reply << pid << p.uname << p.pos.x << p.pos.y;
                }

                auto status = sock.send(reply, addr, port);
            } break;
            case PacketType::Input: {
                uint32_t id;
                float vx, vy;
                pkt >> id >> vx >> vy;

                auto it = players.find(id);
                if (it != players.end()) it->second.vel = {vx, vy};
            } break;
            default: break;
        }
    }
}

void Server::update() {
    for (auto& [id, ps] : players) {
        // authoritative movement
        ps.pos += ps.vel * TICK_SECS; // velocity: px/s multiplying that by second gives us raw pixels
    }

    for (auto& [id, remote] : remotes) {
        sync(remote);
    }
    next_seq++;
}
