#include "server.h"
#include <iostream>
#include <spdlog/spdlog.h>

using namespace std::chrono;
void Server::bind(unsigned short port) {
    if (sock.bind(port) != sf::Socket::Status::Done) {
        spdlog::get("server")->info("Failed to bind to port {}", port);
    }

    sock.setBlocking(false);
    spdlog::get("server")->info("Bound to port {} at {}Hz tickrate", port, TICKRATE);
}

void Server::run() {
    const auto tick_len = milliseconds(1000 / TICKRATE);

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

    pkt << (uint8_t) PacketType::Snapshot << next_seq;

    auto center_chunk = worldToChunkCoords(remote.state.pos);
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
    for (auto const& pl : nearby) pkt << (PlayerState) pl;

    auto status = sock.send(pkt, remote.addr, remote.port);
}

void Server::recv() {
    sf::Packet pkt;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    // receive loop (non-blocking)
    while (sock.receive(pkt, sender, port) == sf::Socket::Status::Done) {
        if (!sender) continue;
        sf::IpAddress addr = *sender;

        uint8_t type_b;
        if (!(pkt >> type_b)) continue;
        PacketType type = static_cast<PacketType>(type_b);

        switch (type) {
            case PacketType::JoinRequest: {
                std::string uname;
                pkt >> uname;

                uint32_t id = next_id++;
                PlayerState state;
                state.id = id;
                state.uname = uname;
                state.pos = {0.f, 0.f};
                state.vel = {0.f, 0.f};
                state.last_seen = steady_clock::now();

                players[id] = state;
                remotes.insert_or_assign(id, Remote{addr, port, state});

                spdlog::get("server")->info("Player '{}' joined (id={}) from {}:{}", uname, id, addr.toString(), port);

                sf::Packet reply;
                reply << (uint8_t) PacketType::JoinAccept << id << (uint32_t) players.size();
                for (auto const& [pid, ps] : players) {
                    reply << pid << ps.uname << ps.pos.x << ps.pos.y;
                }
                auto res = sock.send(reply, addr, port);

                sf::Packet spawn;
                spawn << (uint8_t) PacketType::Snapshot << next_seq;
                spawn << (uint32_t)1; // one player
                spawn << players[id]; // the new player

                for (auto& [oid, remote] : remotes) {
                    if (oid == id) continue; // don’t send to self
                    auto res2 = sock.send(spawn, remote.addr, remote.port);
                }

            } break;

            case PacketType::Input: {
                // client input: id, input_seq, pos.x,pos.y, vel.x,vel.y
                uint32_t id;
                float vx, vy;
                float rot;
                uint16_t held;
                if (!(pkt >> id >> vx >> vy >> rot >> held)) break;

                auto it = players.find(id);
                if (it != players.end()) {
                    it->second.last_seen = steady_clock::now();
                    it->second.vel = {vx, vy};
                    it->second.held_item = held;
                    it->second.rotation = rot;
                } else {
                    // unknown id: could log or ignore
                    spdlog::get("server")->warn("Received Input for unknown id {}", id);
                }
            } break;

            case PacketType::ChatMessage: {
                std::string msg;
                std::string author;
                pkt >> msg;
                if (!msg.empty()) {
                    if (auto remote_id = findRemoteIdByEndpoint(addr, port)) {
                        if (remotes.find(*remote_id) != remotes.end()) {
                            author = remotes.at(*remote_id).state.uname;
                            if (!author.empty()) {
                                sf::Packet chatter;
                                chatter << (uint8_t) PacketType::ChatMessage << std::string("<" + author + "> " + msg);
                                for (auto const& [i, r] : remotes) auto res = sock.send(chatter, r.addr, r.port);
                            }
                        }
                    }
                }
            } break;
            default: break;
        }
    }
}

void Server::update() {
    for (auto& [id, ps] : players) {
        ps.pos += ps.vel * TICK_SECS;

        auto it = remotes.find(id);
        if (it != remotes.end()) it->second.state = ps;
    }

    for (auto& [id, remote] : remotes) {
        sync(remote);
    }
    next_seq++;

    const float STALE_TIMEOUT = 5.f; // seconds without input before removal
    auto now = std::chrono::steady_clock::now();

    std::vector<uint32_t> to_remove;

    for (auto const& [id, ps] : players) {
        auto elapsed = std::chrono::duration<float>(now - ps.last_seen).count();
        if (elapsed > STALE_TIMEOUT) {
            to_remove.push_back(id);
        }
    }

    for (auto id : to_remove) {
        spdlog::get("server")->info("Removing stale player id={}", id);
        remotes.erase(id);
        players.erase(id);
    }
}

std::optional<uint32_t> Server::findRemoteIdByEndpoint(const sf::IpAddress& a, unsigned short port) const {
    for (auto const& [id, r] : remotes) {
        if (r.addr == a && r.port == port) return id;
    }
    return std::nullopt;
}
