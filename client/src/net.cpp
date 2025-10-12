#include "net.h"
#include "game.h"

using namespace std::chrono;

ClientNetwork* ClientNetwork::instance = nullptr;

ClientNetwork* ClientNetwork::getInstance() {
    if (!instance) instance = new ClientNetwork();
    return instance;
}

ClientNetwork::ClientNetwork() : server_addr(0) {
    socket.setBlocking(false);
    start_time = steady_clock::now();
}

ClientNetwork::~ClientNetwork() {
    socket.unbind();
}

void ClientNetwork::join(sf::IpAddress const& ip, unsigned short const& port, sf::String const& uname) {
    server_addr = ip;
    server_port = port;
    username = uname.toAnsiString();

    if (socket.bind(sf::Socket::AnyPort) != sf::Socket::Status::Done) {
        std::puts("OS failed to provide the game a free UDP port to bind to");
        return;
    }

    sf::Packet pkt;
    pkt << PacketHeader{next_seq++, PacketType::JoinRequest} << username;
    auto send = socket.send(pkt, server_addr, server_port);
    last_snapshot = steady_clock::now();
}

void ClientNetwork::update(sf::Time elapsed) {
    sf::Packet pkt;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    // phase 1: process packets
    while (socket.receive(pkt, sender, port) == sf::Socket::Status::Done) {
        if (!sender) continue;
        auto addr = sender.value();
        proc_packets(pkt, addr, port);
    }

    // phase 2: prediction
    float dt = elapsed.asSeconds();
    auto now = steady_clock::now();

    // phase 3: mark stale
    std::vector<uint32_t> to_erase;
    for (auto const& [id, rp] : players) {
        float age = duration<float>(now - rp.last_seen).count();
        if (age > stale_timeout_secs) to_erase.push_back(id);
    }

    // phase 4: remove stale
    for (auto id : to_erase) players.erase(id);

    // phase 5: maintain awareness of connection
    if (duration<float>(steady_clock::now() - last_snapshot).count() > 10.f) {
        Game::getInstance()->sendWarning("Have not received an udpate for over 10 seconds");
    }
}

void ClientNetwork::syncInput(sf::Vector2f const& vel, float rot, uint16_t held) {
    if (my_id == 0) return;

    sf::Packet pkt;
    pkt << PacketHeader{next_seq++, PacketType::Input} << my_id << vel.x << vel.y << rot << held;
    auto res = socket.send(pkt, server_addr, server_port);
}

void ClientNetwork::chat(std::string const& msg) {
    sf::Packet pkt;
    pkt << PacketHeader{next_seq++, PacketType::ChatMessage} << msg;
    auto res = socket.send(pkt, server_addr, server_port);
}

void ClientNetwork::proc_packets(sf::Packet& pkt, const sf::IpAddress& sender, unsigned short port) {
    PacketHeader hdr;
    if (!(pkt >> hdr)) return;
    auto type = hdr.type;

    if (hdr.seq <= last_seq_seen) last_seq_seen = hdr.seq;

    switch (type) {
        case PacketType::JoinAccept: {
            uint32_t count;
            uint32_t server_id;
            pkt >> server_id >> count;
            my_id = server_id;

            for (uint32_t i = 0; i < count; ++i) {
                uint32_t n_id;
                std::string n_uname;
                float nx, ny;
                pkt >> n_id >> n_uname >> nx >> ny;

                if (n_id == my_id) continue;

                Remote r;
                r.state.id = n_id;
                r.state.uname = n_uname;
                r.state.pos = {nx, ny};
                r.state.vel = {0.f, 0.f};
                r.predicted_pos = r.state.pos;
                r.last_seen = steady_clock::now();
                players.insert_or_assign(n_id, std::move(r));
            }
        } break;

        case PacketType::Snapshot: {
            uint64_t time;
            uint32_t count;
            if (!(pkt >> time >> count)) return;
            last_snapshot = steady_clock::now();

            Game::getInstance()->getWorld()->setTime(time);

            for (uint32_t i = 0; i < count; ++i) {
                PlayerState st;
                if (!(pkt >> st)) break;

                if (st.id == my_id) continue; // skip self

                auto it = players.find(st.id);
                if (it == players.end()) {
                    // new remote player
                    Remote rp;
                    rp.state = st;
                    rp.predicted_pos = st.pos;
                    rp.last_seen = std::chrono::steady_clock::now();
                    players.emplace(st.id, std::move(rp));
                } else {
                    it->second.last_seen = std::chrono::steady_clock::now();
                    it->second.state = st;
                }
            }
        } break;
        case PacketType::ChatMessage: {
            std::string content;
            pkt >> content;
            if (!content.empty()) Game::getInstance()->tell(content);
        } break;
        case PacketType::WarningMessage: {
            std::string content;
            pkt >> content;
            if (!content.empty()) Game::getInstance()->sendWarning(content);
        } break;
        case PacketType::ChunkData: {
            auto chunk = Chunk::fromPacket(pkt);
            Game::getInstance()->getWorld()->addChunk(std::move(chunk));
        } break;
        case PacketType::Teleport: {
            float x, y;
            pkt >> x >> y;

            Game::getInstance()->getWorld()->getPlayer()->setPosition({x, y});
            Game::getInstance()->getWorld()->rebakeLighting();
            Game::getInstance()->getWorld()->rebakeLighting();
        } break;
        default: break;
    }
}

void ClientNetwork::requestChunk(sf::Vector2i const& loc) {
    sf::Packet packet;
    packet << PacketHeader{next_seq++, PacketType::ChunkRequest};
    packet << loc.x << loc.y;
    auto discard = socket.send(packet, server_addr, server_port);
}