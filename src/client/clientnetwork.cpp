#include "clientnetwork.hpp"
#include <thread>
#include <chrono>

ClientNetwork::ClientNetwork() {
    cinfo("Client has initialized");
}

void ClientNetwork::connect(std::string const& ip, unsigned short port) {
    if (socket.connect(ip.c_str(), port) != sf::Socket::Done) {
        cerror("Could not connect to the server");
    } else {
        isConnected = true;
        cinfo("Connected to " + ip + ":" + std::to_string(port));
    }
}

void ClientNetwork::receivePackets(sf::TcpSocket* sock) {
    while (true) {
        if (sock->receive(lastReceivedPacket) == sf::Socket::Done) {
            cinfo("Received a packet");
        }

        std::this_thread::sleep_for((std::chrono::milliseconds)25);
    }
}

void ClientNetwork::sendPacket(sf::Packet& packet) {
    if (packet.getDataSize() > 0 && socket.send(packet) != sf::Socket::Done)
    {
        cinfo("Could not send packet");
    }
}

void ClientNetwork::run()
{
    std::thread reception_thred(&ClientNetwork::receivePackets, this, &socket);
}