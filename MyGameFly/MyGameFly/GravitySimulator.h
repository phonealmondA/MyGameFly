#pragma once
#include "Planet.h"
#include "Rocket.h"
#include "VectorHelper.h"
#include <vector>

class GravitySimulator {
private:
    std::vector<Planet*> planets;
    std::vector<Rocket*> rockets;
    const float G = 100000.0f; // Simplified gravitational constant for game physics
    bool simulatePlanetGravity = true; // Flag to control planet-to-planet gravity

public:
    void addPlanet(Planet* planet);
    void addRocket(Rocket* rocket);
    void update(float deltaTime);
    void clearRockets(); // Method to clear rockets list
    void addRocketGravityInteractions(float deltaTime);

    // Method to get the planets list
    const std::vector<Planet*>& getPlanets() const { return planets; }

    // Enable/disable gravity between planets
    void setSimulatePlanetGravity(bool enable) { simulatePlanetGravity = enable; }
};