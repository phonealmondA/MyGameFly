#include "GravitySimulator.h"

void GravitySimulator::addPlanet(Planet* planet)
{
    planets.push_back(planet);
}

void GravitySimulator::addRocket(Rocket* rocket)
{
    rockets.push_back(rocket);
}

void GravitySimulator::clearRockets()
{
    rockets.clear();
}

void GravitySimulator::addRocketGravityInteractions(float deltaTime)
{
    // Apply gravity between rockets
    for (size_t i = 0; i < rockets.size(); i++) {
        for (size_t j = i + 1; j < rockets.size(); j++) {
            Rocket* rocket1 = rockets[i];
            Rocket* rocket2 = rockets[j];

            sf::Vector2f direction = rocket2->getPosition() - rocket1->getPosition();
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            // Minimum distance to prevent extreme forces when very close
            const float minDistance = 10.0f;
            if (distance < minDistance) {
                distance = minDistance;
            }

            // Apply inverse square law for gravity
            float forceMagnitude = G * rocket1->getMass() * rocket2->getMass() / (distance * distance);

            sf::Vector2f normalizedDir = normalize(direction);
            sf::Vector2f accel1 = normalizedDir * forceMagnitude / rocket1->getMass();
            sf::Vector2f accel2 = -normalizedDir * forceMagnitude / rocket2->getMass();

            rocket1->setVelocity(rocket1->getVelocity() + accel1 * deltaTime);
            rocket2->setVelocity(rocket2->getVelocity() + accel2 * deltaTime);
        }
    }
}

void GravitySimulator::update(float deltaTime)
{
    // Apply gravity between planets if enabled
    if (simulatePlanetGravity) {
        for (size_t i = 0; i < planets.size(); i++) {
            for (size_t j = i + 1; j < planets.size(); j++) {
                Planet* planet1 = planets[i];
                Planet* planet2 = planets[j];

                // Skip the first planet (index 0) - it's pinned in place
                if (i == 0) {
                    // Only apply gravity from planet1 to planet2
                    sf::Vector2f direction = planet1->getPosition() - planet2->getPosition();
                    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (distance > planet1->getRadius() + planet2->getRadius()) {
                        float forceMagnitude = G * planet1->getMass() * planet2->getMass() / (distance * distance);
                        sf::Vector2f normalizedDir = normalize(direction);
                        sf::Vector2f accel2 = normalizedDir * forceMagnitude / planet2->getMass();
                        planet2->setVelocity(planet2->getVelocity() + accel2 * deltaTime);
                    }
                }
                else {
                    // Regular gravity calculation between other planets
                    sf::Vector2f direction = planet2->getPosition() - planet1->getPosition();
                    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                    if (distance > planet1->getRadius() + planet2->getRadius()) {
                        float forceMagnitude = G * planet1->getMass() * planet2->getMass() / (distance * distance);
                        sf::Vector2f normalizedDir = normalize(direction);
                        sf::Vector2f accel1 = normalizedDir * forceMagnitude / planet1->getMass();
                        sf::Vector2f accel2 = -normalizedDir * forceMagnitude / planet2->getMass();
                        planet1->setVelocity(planet1->getVelocity() + accel1 * deltaTime);
                        planet2->setVelocity(planet2->getVelocity() + accel2 * deltaTime);
                    }
                }
            }
        }
    }

    // Apply gravity from planets to rockets
    for (auto rocket : rockets) {
        for (auto planet : planets) {
            sf::Vector2f direction = planet->getPosition() - rocket->getPosition();
            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

            // Avoid division by zero and very small distances
            if (distance > planet->getRadius() + 10.0f) {
                // Apply inverse square law for gravity
                // F = G * (m1 * m2) / r^2
                float forceMagnitude = G * planet->getMass() * rocket->getMass() / (distance * distance);

                // Apply Newton's 2nd law: a = F/m
                sf::Vector2f acceleration = normalize(direction) * forceMagnitude / rocket->getMass();

                // Modify rocket velocity based on gravitational acceleration
                sf::Vector2f velocityChange = acceleration * deltaTime;
                sf::Vector2f newVelocity = rocket->getVelocity() + velocityChange;
                rocket->setVelocity(newVelocity);
            }
        }
    }

    // Add rocket-to-rocket gravity interactions
    addRocketGravityInteractions(deltaTime);
}