#pragma once
#include "Planet.h"
#include "Rocket.h"
#include "VectorHelper.h"
#include <vector>

class GravitySimulator {
private:
    std::vector<Planet*> planets;
    std::vector<Rocket*> rockets;
    const float G = 6.67430e-11f * 1000000; // Gravitational constant (scaled for game)

public:
    void addPlanet(Planet* planet);
    void addRocket(Rocket* rocket);
    void update(float deltaTime);
    void clearRockets(); // New method to clear rockets list
};