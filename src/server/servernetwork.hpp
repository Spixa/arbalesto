#pragma once

#include <vector>
#include <SFML/Network.hpp>
#include <string>
#include "clients.hpp"
#include <memory>
#include "../logger.hpp"

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
public:
    Server(unsigned short listenPort);
    Server(Server const&) = delete;
    Server& operator=(Server const&) = delete;

    void run();
public:
    void connectClients(std::vector<Client*>*);
    void disconnectClient(Client*, size_t);

    void receivePackets(Client*, size_t);
    void broadcastPacket(sf::Packet&, sf::IpAddress, unsigned short);
    void managePackets();
};