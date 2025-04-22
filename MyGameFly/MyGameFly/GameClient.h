// GameClient.h
#pragma once
#include "GravitySimulator.h"
#include "VehicleManager.h"
#include "GameState.h"
#include "PlayerInput.h"
#include <vector>
#include <map>

class GameClient {
private:
    GravitySimulator simulator;
    std::vector<Planet*> planets;
    std::map<int, VehicleManager*> remotePlayers;
    VehicleManager* localPlayer;
    int localPlayerId;

    // Last received state for interpolation
    GameState lastState;
    float stateTimestamp;

public:
    GameClient();
    ~GameClient();

    void initialize();
    void update(float deltaTime);
    void processGameState(const GameState& state);
    PlayerInput getLocalPlayerInput(float deltaTime) const;

    void setLocalPlayerId(int id) { localPlayerId = id; }
    int getLocalPlayerId() const { return localPlayerId; }

    VehicleManager* getLocalPlayer() { return localPlayer; }
    const std::vector<Planet*>& getPlanets() const { return planets; }
    const std::map<int, VehicleManager*>& getRemotePlayers() const { return remotePlayers; }
};