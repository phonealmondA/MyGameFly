#include "GravitySimulator.h"

void GravitySimulator::addPlanet(Planet* planet)
{
    planets.push_back(planet);
}

void GravitySimulator::addRocket(Rocket* rocket)
{
    rockets.push_back(rocket);
}

void GravitySimulator::update(float deltaTime)
{
    // Apply gravity from planets to rockets
    for (auto rocket : rockets) {
        for (auto planet : planets) {
            sf::Vector2f direction = planet->getPosition() - rocket->getPosition();
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            // Avoid division by zero and very small distances
            if (distance > planet->getRadius() + 10.0f) {
                float force = G * planet->getMass() / (distance * distance);
                sf::Vector2f acceleration = normalize(direction) * force;

                // Modify rocket velocity based on gravitational acceleration
                sf::Vector2f velocityChange = acceleration * deltaTime;
                sf::Vector2f newVelocity = rocket->getVelocity() + velocityChange;
                rocket->setVelocity(newVelocity);
            }
        }
    }
}