#include "servernetwork.hpp"
#include <sstream>
#include <thread>

Server::Server(unsigned short port) : listenPort(port), version("0.1-beta"), local()
{
    sinfo("Arbalesto server v" + version + " has begun");
    sinfo("Arbalesto server is based on openSIMP v1 schema");
    swarn("Server is not intended to be used yet");
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
            
            local.join(server::Player("hi" , {0,0}));
            sf::Packet join;
            join << net::Packet::PlayerJoinPacket;

            broadcastPacket(join, sf::IpAddress(), 0);

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
        sinfo("Received new message from " + sock->getRemoteAddress().toString());
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
    std::thread connectionThr{&Server::connectClients, this, &clients};

    managePackets();
}