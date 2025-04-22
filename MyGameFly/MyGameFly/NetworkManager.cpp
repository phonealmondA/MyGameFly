// NetworkManager.cpp
#include "NetworkManager.h"
#include <iostream>

NetworkManager::NetworkManager() : isHost(false), port(0), connected(false) {
    // Initialize network components
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::hostGame(unsigned short port) {
    this->port = port;
    isHost = true;

    // Start listening for connections
    if (listener.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        return false;
    }

    listener.setBlocking(false);
    connected = true;
    return true;
}

bool NetworkManager::joinGame(const sf::IpAddress& address, unsigned short port) {
    isHost = false;

    // Connect to the server
    if (serverConnection.connect(address, port) != sf::Socket::Status::Done) {
        std::cerr << "Failed to connect to " << address << ":" << port << std::endl;
        return false;
    }

    serverConnection.setBlocking(false);
    connected = true;
    return true;
}

void NetworkManager::disconnect() {
    if (isHost) {
        listener.close();

        // Disconnect all clients
        for (auto client : clients) {
            client->disconnect();
            delete client;
        }
        clients.clear();
    }
    else {
        serverConnection.disconnect();
    }

    connected = false;
}

void NetworkManager::update() {
    if (!connected) return;

    if (isHost) {
        // Accept new connections
        sf::TcpSocket* newClient = new sf::TcpSocket();
        if (listener.accept(*newClient) == sf::Socket::Status::Done) {
            newClient->setBlocking(false);
            clients.push_back(newClient);
            std::cout << "New client connected: " << clients.size() << std::endl;
        }
        else {
            delete newClient;
        }

        // Check for messages from clients
        for (size_t i = 0; i < clients.size(); ++i) {
            sf::Packet packet;
            if (clients[i]->receive(packet) == sf::Socket::Status::Done) {
                // Parse player input from packet
                PlayerInput input;
                packet >> input;

                if (onPlayerInputReceived) {
                    onPlayerInputReceived(i, input);
                }
            }
        }
    }
    else {
        // Check for messages from server
        sf::Packet packet;
        if (serverConnection.receive(packet) == sf::Socket::Status::Done) {
            // Parse game state from packet
            GameState state;
            packet >> state;

            if (onGameStateReceived) {
                onGameStateReceived(state);
            }
        }
    }
}

bool NetworkManager::sendGameState(const GameState& state) {
    if (!isHost || !connected) return false;

    sf::Packet packet;
    packet << state;

    bool allSucceeded = true;
    for (auto client : clients) {
        if (client->send(packet) != sf::Socket::Status::Done) {
            allSucceeded = false;
        }
    }

    return allSucceeded;
}

bool NetworkManager::sendPlayerInput(const PlayerInput& input) {
    if (isHost || !connected) return false;

    sf::Packet packet;
    packet << input;

    return (serverConnection.send(packet) == sf::Socket::Status::Done);
}