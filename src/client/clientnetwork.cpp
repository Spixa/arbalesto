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
                float dX, dY;
                net::TeleportReason tpReason;

                lastReceivedPacket >> dX >> dY >> tpReason;

                Game::getInstance()
                    ->getStateManager()
                    ->getGameState()
                    ->getPlayer()
                    ->move(dX, dY);

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
                
                auto pos = Game::getInstance()->getStateManager()->getGameState()->getPlayer()->getPosition();
                cinfo("Client's player was teleported to new location [" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + "] because of \"" + tpReasonStr + "\"");
            } else
            if (p == net::Packet::UpdatePositionPacket) {
                float newX, newY;
                std::string name;
                
                lastReceivedPacket >> name >> newX >> newY;

                for (auto x : *Game::getInstance()
                    ->getStateManager()
                    ->getGameState()
                    ->getPlayers())
                {
                    if (x->getDisplayName() == name) {
                        x->setPosition(newX, newY);
                    }
                }

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