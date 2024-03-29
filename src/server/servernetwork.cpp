#include "servernetwork.hpp"
#include <sstream>
#include <cmath>
#include <thread>


// spec


Server::Server(unsigned short port) : listenPort(port), version("0.1-beta"), local()
{
    sinfo("Arbalesto server v" + version + " has begun");
    sinfo("Arbalesto server is based on openSIMP v1 schema");
    if (listener.listen(listenPort) != sf::Socket::Done) {
        sinfo("Cannot listen on port " + std::to_string(listenPort));
    }
}

Server::~Server() {
    sinfo("Stopping Arbalesto server...");

    for (auto x : clients) {
        x->socket()->disconnect();
        delete(x);
    }

    clients.clear();
}

void Server::connectClients(std::vector<Client*> *clients) {
    while (true) {
        sf::TcpSocket* newClient = new sf::TcpSocket();

        if (listener.accept(*newClient) == sf::Socket::Done)
        {
            newClient->setBlocking(false);
            clients->push_back(new Client(newClient));

            std::stringstream msg{};
            msg << "Added client " << newClient->getRemoteAddress() << ":" << newClient->getRemotePort() << " on slot " << clients->size();
            sinfo(msg.str());

            strace("connectClients(): Posted task");

        } else {
            serror("Server listener error, restart the server");
            delete newClient;
            break;
        }
    }
}

void Server::broadcastPacket(sf::Packet& packet, sf::IpAddress excludeAddr = {}, unsigned short excludePort = 0) {
    for (size_t iterator = 0; iterator < clients.size(); iterator++) {
        sf::TcpSocket* socket = clients[iterator]->socket();

        if (socket->getRemoteAddress() != excludeAddr || socket->getRemotePort() != excludePort) {
            if (socket->send(packet) != sf::Socket::Done) {
                serror("Could not send packet on broadcast");
            }
        }
    }
}

void Server::disconnectClient(Client* clientPtr, size_t pos) {
    std::stringstream msg{};
    msg << "Client " << clientPtr->socket()->getRemoteAddress() << ":" << clientPtr->socket()->getRemotePort() << " disconnected";
    sinfo(msg.str());

    clientPtr->socket()->disconnect();
    delete(clientPtr);

    clients.erase(clients.begin() + pos);
}

void Server::receivePackets(Client* client, size_t iterator) {
    sf::Packet packet;
    sf::TcpSocket* sock = client->socket();

    if (sock->receive(packet) == sf::Socket::Disconnected) {
        disconnectClient(client, iterator);
    }

    if (packet.getDataSize() > 0)
    {
        net::Packet receivedPacket;
        packet >> receivedPacket;

        switch (receivedPacket) {
            case net::Packet::ClientNickPacket: {
                std::string name;
                packet >> name;

                if (!std::regex_match(name, alphanum) && name.length() <= 16) {
                    sf::Packet kickPacket;
                    kickPacket << net::Packet::KickClientPacket << net::KickReason::BadUsernameKick;
                    client->socket()->send(kickPacket);
                    disconnectClient(client, iterator);

                    sf::Packet announceKick;
                    announceKick << net::Packet::ServerBroadcastPacket;
                    std::string kickMessage = "\"" + name + "\" was kicked from the server for non-alphanumeric or a long username";
                    announceKick << kickMessage; 
                    broadcastPacket(announceKick);
                    return;
                }

                for (auto x : clients) {
                    if (x->getName() == name) {
                        sf::Packet kickPacket;
                        kickPacket << net::Packet::KickClientPacket << net::KickReason::UsernameTakenKick;
                        client->socket()->send(kickPacket);
                        disconnectClient(client, iterator);

                        // TOOD: move block to kickClient(Client, ...) 
                        sf::Packet announceKick;
                        announceKick << net::Packet::ServerBroadcastPacket;
                        std::string kickMessage = "\"" + name + "\" was kicked from the server for having a taken username";
                        announceKick << kickMessage; 

                        broadcastPacket(announceKick);
                        return;
                    }
                }


                sf::Packet welcomePacket;
                welcomePacket << net::Packet::WelcomePacket << welcomeMsg;

                client->socket()->send(welcomePacket);

                sf::Packet updatePlList;
                updatePlList << net::Packet::UpdatePlayerListPacket;
                std::vector<Client*> plClients = clients;
                plClients.erase(plClients.begin() + iterator);

                updatePlList << int(plClients.size());

                for (auto x : plClients) {
                    updatePlList << x->getName() << x->getPosition().x << x->getPosition().y;
                }
                client->socket()->send(updatePlList);


                client->setName(name);
                sf::Vector2f startingPosition = {500.f, 500.f};

                sf::Packet join;
                join << net::Packet::PlayerJoinPacket << name << startingPosition.x << startingPosition.y;

                broadcastPacket(join, client->socket()->getRemoteAddress(), client->socket()->getRemotePort());

                local.join(server::Player(name , {0,0}));
            } break;
            case net::Packet::PingPacket: {
                sinfo(client->getName() + " pinged the server");
            } break;
            case net::Packet::ClientMovementPacket: {
                float newX, newY;

                packet >> newX >> newY;

                float dX = client->getPosition().x - newX;
                float dY = client->getPosition().y - newY;

                if (std::abs(dX) <=14.f && std::abs(dY) <=14.f) {
                    client->setPosition({newX, newY});
                    strace("> " + client->getName() + " moved to [" + std::to_string(newX) + ", " + std::to_string(newY) + "]"
                    "(dX: " + std::to_string(int(std::floor(dX))) + ", dY: " + std::to_string(int(std::floor(dY))) + ")");
                }
                else {
                    strace("> " + client->getName() + " moved too far! "
                    "(edX: " + std::to_string(dX) + ", edY: " + std::to_string(dY) + ")");
                    
                    sf::Packet tpBackPacket;
                    tpBackPacket << net::Packet::TeleportPlayerPacket << dX << dY << net::TeleportReason::AnticheatTeleport;

                    client->socket()->send(tpBackPacket);
                }

                sf::Packet updatePosition;
                updatePosition << net::Packet::UpdatePositionPacket << client->getName() << dX << dY;
                
                broadcastPacket(updatePosition, client->socket()->getRemoteAddress(), client->socket()->getRemotePort());

            } break;
            case net::Packet::UpdateAnimationPacket: {
                
            } break;
            default: {
                sinfo("Received an illegal packet from the client");
            }
        }
    }
}

void Server::managePackets()
{
    while(true)
    {
        for (size_t iterator = 0; iterator < clients.size(); iterator++) {
            receivePackets(clients[iterator], iterator);
        }
        
        local.update();
        std::this_thread::sleep_for((std::chrono::milliseconds)1);
    }
}

void Server::run() 
{
    sinfo("(@connection) Thread launched");
    std::thread connectionThr{&Server::connectClients, this, &clients};

    sinfo("(@reception) Thread launched");
    managePackets();
}