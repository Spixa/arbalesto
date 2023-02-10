#pragma once

// sfml
#include <SFML/Network.hpp>

// std
#include <vector>
#include <string>
#include <memory>

// serverside inclusions
#include "clients.hpp"
#include "server.hpp"

// logger
#include "../logger.hpp"

// logging macros
#define strace(x) log("server").trace(x)
#define sinfo(x) log("server").info(x)
#define swarn(x) log("server").warn(x)
#define serror(x) log("server").error(x)

class Server {
private:
    unsigned short listenPort;
    sf::TcpListener listener;
    const std::string version;

    std::vector<Client*> clients;
    LocalServer local;
public:
    Server(unsigned short listenPort);
    Server(Server const&) = delete;
    Server& operator=(Server const&) = delete;
    virtual ~Server();

    void run();
public:
    void connectClients(std::vector<Client*>*);
    void disconnectClient(Client*, size_t);

    void receivePackets(Client*, size_t);
    void broadcastPacket(sf::Packet&, sf::IpAddress, unsigned short);
    void managePackets();
};