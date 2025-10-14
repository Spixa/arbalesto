#pragma once
#include "util.h"
#include <SFML/Network.hpp>
#include <memory>
#include "packets.h"

struct Remote {
    PlayerState state;
    sf::Vector2f predicted_pos;
    sf::Vector2f velocity;
    std::chrono::steady_clock::time_point last_seen;
};
class ClientNetwork : NonCopyable {
public:
    void join(sf::IpAddress const& ip, unsigned short const& port, sf::String const& username);
    void update(sf::Time elapsed);
    void chat(std::string const& msg);
    void syncInput(sf::Vector2f const& vel, float rot, uint16_t held);
    void submitTile(sf::Vector2i const& chunk, sf::Vector2u const& local, Tile tile);
    void requestChunk(sf::Vector2i const& pos);
    const std::unordered_map<uint32_t, Remote>& getPlayers() const { return players; }

    uint32_t getMyId() const { return my_id; }
    int getPingMs() const;
    static ClientNetwork* getInstance();
private:
    ClientNetwork();
    ~ClientNetwork();
private:
    static ClientNetwork* instance;
private:
    sf::UdpSocket socket;

    // cache
    sf::IpAddress server_addr;
    uint16_t server_port;
    std::string username;
    std::chrono::steady_clock::time_point last_ping;
    uint32_t my_id{0};

    std::unordered_map<uint32_t, Remote> players;
    std::chrono::steady_clock::time_point start_time, last_snapshot;
    int ping_ms{0};
    uint32_t last_seq_seen;
    uint32_t next_seq = 1;

    const float stale_timeout_secs = 2.5f;
    void proc_packets(sf::Packet& pkt, const sf::IpAddress& sender, unsigned short port);
};