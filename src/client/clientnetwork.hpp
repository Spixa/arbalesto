#pragma once
// sfml
#include <SFML/Network.hpp>

// std
#include <vector>
#include <string>
#include <memory>
#include <thread>

#include "../packets.hpp"

// logger
#include "../logger.hpp"

// logging macros
#define ctrace(x) log("client").trace(x)
#define cinfo(x) log("client").info(x)
#define cwarn(x) log("client").warn(x)
#define cerror(x) log("client").error(x)

class ClientNetwork {
    sf::TcpSocket socket;
    sf::Packet lastReceivedPacket;
    bool isConnected = false;
    std::thread reception_thred;
public:
    ClientNetwork();
private:
    void receivePackets(sf::TcpSocket*);
public:
    void connect(const std::string& ip, unsigned short port);
    void sendPacket(sf::Packet& packet);
public:
    void run();

};