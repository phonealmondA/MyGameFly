// NetworkManager.cpp
#include "NetworkManager.h"
#include "GameServer.h"
#include "GameClient.h"
#include <iostream>

// Message types for network communication
enum MessageType {
    MSG_GAME_STATE = 1,
    MSG_PLAYER_INPUT = 2,
    MSG_PLAYER_ID = 3
};

NetworkManager::NetworkManager() : isHost(false), port(0), connected(false), gameServer(nullptr), gameClient(nullptr) {
    // Initialize network components
}

NetworkManager::~NetworkManager() {
    disconnect();
}

void NetworkManager::setGameServer(GameServer* server) {
    gameServer = server;
}

void NetworkManager::setGameClient(GameClient* client) {
    gameClient = client;
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

            // Create a unique ID for the client (use client index + 1 to avoid ID 0)
            int clientId = clients.size(); // This will be 1 for the first client

            // Send acknowledgment with player ID to the client
            sf::Packet idPacket;
            idPacket << static_cast<uint32_t>(MSG_PLAYER_ID) << static_cast<uint32_t>(clientId);
            newClient->send(idPacket);

            // Create a new player for this client
            if (gameServer) {
                sf::Vector2f spawnPos = gameServer->getPlanets()[0]->getPosition() +
                    sf::Vector2f(0, -(gameServer->getPlanets()[0]->getRadius() + GameConstants::ROCKET_SIZE + 30.0f));
                gameServer->addPlayer(clientId, spawnPos, sf::Color::Red);
            }

            std::cout << "New client connected with ID: " << clientId << std::endl;
        }
        else {
            delete newClient;
        }

        // Check for messages from clients
        for (size_t i = 0; i < clients.size(); ++i) {
            sf::Packet packet;
            if (clients[i]->receive(packet) == sf::Socket::Status::Done) {
                // Parse player input from packet
                uint32_t msgType;
                packet >> msgType;

                if (msgType == MSG_PLAYER_INPUT) {
                    PlayerInput input;
                    packet >> input;

                    if (onPlayerInputReceived) {
                        // Debug output
                        std::cout << "Server received input from player ID: " << input.playerId
                            << " W/Up:" << input.thrustForward
                            << " S/Down:" << input.thrustBackward
                            << " A/Left:" << input.rotateLeft
                            << " D/Right:" << input.rotateRight << std::endl;

                        // Use the playerId from the input packet
                        onPlayerInputReceived(input.playerId, input);
                    }
                }
            }
        }

        // Periodically send updated game state to all clients
        static sf::Clock updateClock;
        if (updateClock.getElapsedTime().asMilliseconds() > 50) { // 20 updates per second
            updateClock.restart();

            if (gameServer) {
                GameState state = gameServer->getGameState();
                sendGameState(state);
            }
        }
    }
    else {
        // Check for messages from server
        sf::Packet packet;
        if (serverConnection.receive(packet) == sf::Socket::Status::Done) {
            uint32_t msgType;
            packet >> msgType;

            if (msgType == MSG_PLAYER_ID) {
                uint32_t playerId;
                packet >> playerId;
                if (gameClient) {
                    gameClient->setLocalPlayerId(static_cast<int>(playerId));
                    std::cout << "Received player ID from server: " << playerId << std::endl;
                }
            }
            else if (msgType == MSG_GAME_STATE) {
                // Parse game state from packet
                GameState state;
                packet >> state;

                if (onGameStateReceived) {
                    onGameStateReceived(state);
                }
            }
        }
    }
}

bool NetworkManager::sendGameState(const GameState& state) {
    if (!isHost || !connected) return false;

    sf::Packet packet;
    packet << static_cast<uint32_t>(MSG_GAME_STATE) << state;

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
    packet << static_cast<uint32_t>(MSG_PLAYER_INPUT) << input;

    return (serverConnection.send(packet) == sf::Socket::Status::Done);
}