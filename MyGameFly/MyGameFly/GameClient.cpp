// GameClient.cpp
#include "GameClient.h"
#include "GameConstants.h"

GameClient::GameClient() : localPlayer(nullptr), localPlayerId(0), stateTimestamp(0.0f) {
}

GameClient::~GameClient() {
    // Clean up players
    for (auto& pair : remotePlayers) {
        delete pair.second;
    }
    remotePlayers.clear();

    delete localPlayer;

    // Clean up planets
    for (auto& planet : planets) {
        delete planet;
    }
    planets.clear();
}

void GameClient::initialize() {
    // Create main planet (placeholder until we get state from server)
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

    // Setup local player (placeholder until we get assigned ID)
    sf::Vector2f initialPos = planets[0]->getPosition() +
        sf::Vector2f(0, -(planets[0]->getRadius() + GameConstants::ROCKET_SIZE));
    localPlayer = new VehicleManager(initialPos, planets);

    // Setup gravity simulator
    simulator.setSimulatePlanetGravity(true);
    for (auto planet : planets) {
        simulator.addPlanet(planet);
    }
    simulator.addVehicleManager(localPlayer);
}

void GameClient::update(float deltaTime) {
    // Update simulator
    simulator.update(deltaTime);

    // Update planets
    for (auto planet : planets) {
        planet->update(deltaTime);
    }

    // Update local player
    if (localPlayer) {
        localPlayer->update(deltaTime);
    }

    // Update remote players
    for (auto& pair : remotePlayers) {
        pair.second->update(deltaTime);
    }
}

void GameClient::processGameState(const GameState& state) {
    // Update last state
    lastState = state;
    stateTimestamp = state.timestamp;

    // Process planets
    for (const auto& planetState : state.planets) {
        // Make sure we have enough planets
        while (planetState.planetId >= static_cast<int>(planets.size())) {
            planets.push_back(new Planet(sf::Vector2f(0, 0), 0, 1.0f));
            simulator.addPlanet(planets.back());
        }

        // Update planet state
        Planet* planet = planets[planetState.planetId];
        planet->setPosition(planetState.position);
        planet->setVelocity(planetState.velocity);
        planet->setMass(planetState.mass);
        // Note: setMass will update radius based on the mass-radius relationship
    }

    // Process rockets
    for (const auto& rocketState : state.rockets) {
        // Skip local player's rocket
        if (rocketState.playerId == localPlayerId) {
            continue;
        }

        // Get or create vehicle manager for this player
        auto it = remotePlayers.find(rocketState.playerId);
        VehicleManager* manager = nullptr;

        if (it == remotePlayers.end()) {
            // Create a new remote player
            manager = new VehicleManager(rocketState.position, planets);
            remotePlayers[rocketState.playerId] = manager;
            simulator.addVehicleManager(manager);
        }
        else {
            manager = it->second;
        }

        // Update rocket state
        Rocket* rocket = manager->getRocket();
        rocket->setPosition(rocketState.position);
        rocket->setVelocity(rocketState.velocity);
        rocket->setRotation(rocketState.rotation);
        rocket->setThrustLevel(rocketState.thrustLevel);
        // Note: mass and color are usually not changed frequently
    }
}

PlayerInput GameClient::getLocalPlayerInput(float deltaTime) const {
    PlayerInput input;
    input.playerId = localPlayerId;
    input.deltaTime = deltaTime;

    // Get keyboard state
    input.thrustForward = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    input.thrustBackward = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    input.rotateLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    input.rotateRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    input.switchVehicle = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L);

    // Get thrust level
    if (localPlayer && localPlayer->getActiveVehicleType() == VehicleType::ROCKET) {
        input.thrustLevel = localPlayer->getRocket()->getThrustLevel();
    }

    return input;
}