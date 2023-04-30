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
                float dX, dY;
                std::string name;
                
                lastReceivedPacket >> name >> dX >> dY;

                for (auto x : *Game::getInstance()
                    ->getStateManager()
                    ->getGameState()
                    ->getPlayers())
                {
                    if (x->getDisplayName() == name) {
                        x->setVelocity({-dX, -dY});
                    }
                }

            } else
            if (p == net::Packet::UpdatePlayerListPacket) {
                int count;
                lastReceivedPacket >> count;

                ctrace("Player count before join: " + std::to_string(count));

                for (int i = 0; i < count; i++) {
                    std::string name;
                    float x, y;

                    lastReceivedPacket >> name >> x >> y;

                    Game::getInstance()
                    ->getStateManager()
                    ->getGameState()
                    ->addPlayer(name, sf::Vector2f(x, y));

                    ctrace("Added player with name \"" + name + "\" at [" + std::to_string(x) + ", " + std::to_string(y) + "]");
                }
            } else
            if (p == net::Packet::KickClientPacket) {
                net::KickReason reason;
                std::string reasonStr;

                lastReceivedPacket >> reason;

                switch (reason) {
                    case net::KickReason::UsernameTakenKick: {
                        reasonStr = "username being taken";
                    } break;
                    case net::KickReason::BadUsernameKick: {
                        reasonStr = "bad username";
                    } break;
                    case net::KickReason::AnticheatKick: {
                        reasonStr = "cheating";
                    } break;
                    default: {
                        reasonStr = "no reason";
                    } break;
                }

                cinfo("Client was kicked out of the server for " + reasonStr);
                exit(0);
            } else
            if (p == net::Packet::ServerBroadcastPacket) {
                std::string content;
                lastReceivedPacket >> content;

                cinfo("Broadcast: " + content);
            } else 
            if (p == net::Packet::WelcomePacket) {
                std::string welcome;
                lastReceivedPacket >> welcome;

                cinfo("Server welcomes you: ");
                std::cout << welcome;
            }
        }

        std::this_thread::sleep_for((std::chrono::milliseconds)1);
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