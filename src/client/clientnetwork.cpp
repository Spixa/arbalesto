#include "clientnetwork.hpp"
#include <thread>
#include <chrono>
#include "../game.hpp"
#include "../player.hpp"

ClientNetwork::ClientNetwork() {
    cinfo("Client has initialized");
}

void ClientNetwork::connect(std::string const& ip, unsigned short port) {
    if (socket.connect(ip.c_str(), port) != sf::Socket::Done) {
        cerror("Could not connect to the server");
        cerror("Exitting the application (bad server)");
        exit(0);
    } else {
        isConnected = true;
        cinfo("Connected to " + ip + ":" + std::to_string(port));
    }
}

void ClientNetwork::receivePackets(sf::TcpSocket* sock) {
    while (true) {
        if (sock->receive(lastReceivedPacket) == sf::Socket::Done) {
            net::Packet p;
            lastReceivedPacket >> p;
            
            if (p == net::Packet::PlayerJoinPacket) {
                std::string name;
                float sX = 0.f, sY = 0.f;
                lastReceivedPacket >> name >> sX >> sY;

                cinfo("'" + name + "' joined the server at [" + std::to_string(sX) + ", " + std::to_string(sY) + "]");
                Game::getInstance()
                    ->getStateManager()
                    ->getGameState()
                    ->addPlayer(name, sf::Vector2f(sX, sY));
            } else
            if (p == net::Packet::TeleportPlayerPacket) {
                float newX, newY;
                net::TeleportReason tpReason;

                lastReceivedPacket >> newX >> newY >> tpReason;

                Game::getInstance()
                    ->getStateManager()
                    ->getGameState()
                    ->getPlayer()
                    ->setPosition(newX, newY);

                std::string tpReasonStr;

                switch (tpReason) {
                    case net::TeleportReason::AnticheatTeleport: {
                        tpReasonStr = "Anticheat";
                    } break;
                    case net::TeleportReason::CommandTeleport: {
                        tpReasonStr = "Server command";
                    } break;
                    default: {
                        tpReasonStr  ="Automatic Server Teleportation";
                    } break;
                }
                
                cinfo("Client's player was teleported to new location [" + std::to_string(newX) + ", " + std::to_string(newY) + "] because of \"" + tpReasonStr + "\"");
            }
        }

        std::this_thread::sleep_for((std::chrono::milliseconds)25);
    }
}

void ClientNetwork::sendPacket(sf::Packet& packet) {
    if (packet.getDataSize() > 0 && socket.send(packet) != sf::Socket::Done)
    {
        cinfo("Could not send packet");
        Game::getInstance()->failedPacketCounter++;
    }
}

void ClientNetwork::run()
{
    reception_thred = std::thread(&ClientNetwork::receivePackets, this, &socket);
}