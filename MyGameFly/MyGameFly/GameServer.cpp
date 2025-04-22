// GameServer.cpp
#include "GameServer.h"
#include "GameConstants.h"

GameServer::GameServer() : sequenceNumber(0), gameTime(0.0f) {
}

GameServer::~GameServer() {
    // Clean up players
    for (auto& pair : players) {
        delete pair.second;
    }
    players.clear();

    // Clean up planets
    for (auto& planet : planets) {
        delete planet;
    }
    planets.clear();
}

void GameServer::initialize() {
    // Create main planet
    Planet* mainPlanet = new Planet(
        sf::Vector2f(GameConstants::MAIN_PLANET_X, GameConstants::MAIN_PLANET_Y),
        0, GameConstants::MAIN_PLANET_MASS, sf::Color::Blue);
    mainPlanet->setVelocity(sf::Vector2f(0.f, 0.f));
    planets.push_back(mainPlanet);

    // Create secondary planet
    Planet* secondaryPlanet = new Planet(
        sf::Vector2f(GameConstants::SECONDARY_PLANET_X, GameConstants::SECONDARY_PLANET_Y),
        0, GameConstants::SECONDARY_PLANET_MASS, sf::Color::Green);
    secondaryPlanet->setVelocity(sf::Vector2f(0.f, GameConstants::SECONDARY_PLANET_ORBITAL_VELOCITY));
    planets.push_back(secondaryPlanet);

    // Setup gravity simulator
    simulator.setSimulatePlanetGravity(true);
    for (auto planet : planets) {
        simulator.addPlanet(planet);
    }
}

int GameServer::addPlayer(int playerId, sf::Vector2f initialPos, sf::Color color) {
    // Check if player already exists
    if (players.find(playerId) != players.end()) {
        return playerId; // Player already exists
    }

    // Create a new vehicle manager for this player
    VehicleManager* manager = new VehicleManager(initialPos, planets);
    manager->getRocket()->setColor(color);

    // Add to simulator
    simulator.addVehicleManager(manager);

    // Store in players map
    players[playerId] = manager;

    std::cout << "Added player with ID: " << playerId << std::endl;
    return playerId;
}

void GameServer::removePlayer(int playerId) {
    auto it = players.find(playerId);
    if (it != players.end()) {
        simulator.removeVehicleManager(it->second);
        delete it->second;
        players.erase(it);
    }
}

void GameServer::update(float deltaTime) {
    // Update game time
    gameTime += deltaTime;

    // Update simulator
    simulator.update(deltaTime);

    // Update planets
    for (auto planet : planets) {
        planet->update(deltaTime);
    }

    // Update all players
    for (auto& pair : players) {
        pair.second->update(deltaTime);
    }

    // Increment sequence number
    sequenceNumber++;
}

void GameServer::handlePlayerInput(int playerId, const PlayerInput& input) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        // Player not found - could be a new connection, create player
        std::cout << "Unknown player ID: " << playerId << ", creating new player" << std::endl;
        sf::Vector2f spawnPos = planets[0]->getPosition() +
            sf::Vector2f(0, -(planets[0]->getRadius() + GameConstants::ROCKET_SIZE));
        players[playerId] = new VehicleManager(spawnPos, planets);
        simulator.addVehicleManager(players[playerId]);
        return;
    }

    // Apply input to the correct player's vehicle manager
    VehicleManager* manager = it->second;

    // Apply input to the vehicle
    if (input.thrustForward) {
        manager->applyThrust(1.0f);
    }
    if (input.thrustBackward) {
        manager->applyThrust(-0.5f);
    }
    if (input.rotateLeft) {
        manager->rotate(-6.0f * input.deltaTime * 60.0f);
    }
    if (input.rotateRight) {
        manager->rotate(6.0f * input.deltaTime * 60.0f);
    }
    if (input.switchVehicle) {
        manager->switchVehicle();
    }

    // Apply thrust level
    if (manager->getActiveVehicleType() == VehicleType::ROCKET) {
        manager->getRocket()->setThrustLevel(input.thrustLevel);
    }
}

GameState GameServer::getGameState() const {
    GameState state;
    state.sequenceNumber = sequenceNumber;
    state.timestamp = gameTime;

    // Add all rockets
    int rocketIndex = 0;
    for (const auto& pair : players) {
        int playerId = pair.first;
        const VehicleManager* manager = pair.second;

        // Only add if it's a rocket
        if (manager->getActiveVehicleType() == VehicleType::ROCKET) {
            const Rocket* rocket = manager->getRocket();

            RocketState rocketState;
            rocketState.playerId = playerId;
            rocketState.position = rocket->getPosition();
            rocketState.velocity = rocket->getVelocity();
            rocketState.rotation = rocket->getRotation();
            rocketState.angularVelocity = 0.0f; // Not tracked in your current design
            rocketState.thrustLevel = rocket->getThrustLevel();
            rocketState.mass = rocket->getMass();
            rocketState.color = rocket->getColor();

            state.rockets.push_back(rocketState);
        }
        // TODO: Add car state if needed
    }

    // Add all planets
    for (size_t i = 0; i < planets.size(); ++i) {
        const Planet* planet = planets[i];

        PlanetState planetState;
        planetState.planetId = static_cast<int>(i);
        planetState.position = planet->getPosition();
        planetState.velocity = planet->getVelocity();
        planetState.mass = planet->getMass();
        planetState.radius = planet->getRadius();
        planetState.color = planet->getColor();

        state.planets.push_back(planetState);
    }

    return state;
}