#pragma once

// sfml
#include <SFML/Network.hpp>

// std
#include <vector>
#include <string>
#include <memory>
#include <regex>

// serverside inclusions
#include "clients.hpp"
#include "server.hpp"
#include "../packets.hpp"

// logger
#include "../logger.hpp"

// logging macros
#define strace(x) log("server").trace(x)
#define sinfo(x) log("server").info(x)
#define swarn(x) log("server").warn(x)
#define serror(x) log("server").error(x)

#ifdef NDEBUG
    #define RELEASE_MODE "RELEASE"
#else
    #define RELEASE_MODE "DEBUG"
#endif

#define BUI 3
#define VER 1 


class Server {
private:
    unsigned short listenPort;
    sf::TcpListener listener;
    const std::string version;

    std::vector<Client*> clients;
    LocalServer local;

    std::string welcomeMsg =    "------------------[Welcome]-------------------\n"
                                "| Welcome to another Arbalesto v1 server      |\n"
                                "| Please enjoy your stay                      |\n"
                                "| Written in C++ and using SFML               |\n"
                                "------------------[Welcome]-------------------\n"
                                "Server is arbalesto-server_v" + std::to_string(VER) + "-bui" + std::to_string(BUI) + "_" + std::string(RELEASE_MODE) + "\n";
    std::regex alphanum{"^[a-zA-Z0-9_]*$"};
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