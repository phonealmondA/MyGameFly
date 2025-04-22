// NetworkManager.h
#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <functional>
#include "GameState.h"
#include "PlayerInput.h"

// Forward declarations
class GameServer;
class GameClient;

class NetworkManager {
private:
    bool isHost;
    std::vector<sf::TcpSocket*> clients;  // For host
    sf::TcpSocket serverConnection;       // For client
    sf::TcpListener listener;             // For host
    unsigned short port;
    bool connected;

    // References to game components
    GameServer* gameServer;
    GameClient* gameClient;

public:
    NetworkManager();
    ~NetworkManager();

    // Set references to game components
    void setGameServer(GameServer* server);
    void setGameClient(GameClient* client);

    bool hostGame(unsigned short port);
    bool joinGame(const sf::IpAddress& address, unsigned short port);
    void disconnect();
    void update();
    bool sendGameState(const GameState& state);   // Host only
    bool sendPlayerInput(const PlayerInput& input); // Client only

    bool isConnected() const { return connected; }
    bool getIsHost() const { return isHost; }

    // Callbacks to be set by the game
    std::function<void(int clientId, const PlayerInput&)> onPlayerInputReceived;
    std::function<void(const GameState&)> onGameStateReceived;
};