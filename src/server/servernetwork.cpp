#include "servernetwork.hpp"
#include <sstream>
#include <cmath>
#include <thread>

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

void Server::broadcastPacket(sf::Packet& packet, sf::IpAddress excludeAddr, unsigned short excludePort) {
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
        // debug("Received new message from " + sock->getRemoteAddress().toString());

        net::Packet receivedPacket;
        packet >> receivedPacket;

        switch (receivedPacket) {
            case net::Packet::ClientNickPacket: {
                std::string name;
                packet >> name;
                client->setName(name);

                sf::Packet join;
                join << net::Packet::PlayerJoinPacket << name;

                broadcastPacket(join, sf::IpAddress(), 0);

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
                    sinfo("(Velocity) " + client->getName() + " moved to [" + std::to_string(newX) + ", " + std::to_string(newY) + "]"
                    "(dX: " + std::to_string(int(std::floor(dX))) + ", dY: " + std::to_string(int(std::floor(dY))) + ")");
                }
                else {
                    sinfo("(Velocity) CHEAT DETECTED >> " + client->getName() + " moved too far!"
                    " . Exact: (dX: " + std::to_string(dX) + ", dY: " + std::to_string(dY) + ")");
                    // tp player back
                }

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
        std::this_thread::sleep_for((std::chrono::milliseconds)25);
    }
}

void Server::run() 
{
    sinfo("(@connection) Thread launched");
    std::thread connectionThr{&Server::connectClients, this, &clients};

    sinfo("(@reception) Thread launched");
    managePackets();
}