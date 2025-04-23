#include "Engine.h"
#include "GameConstants.h"
#include <cmath>

Engine::Engine(sf::Vector2f relPos, float thrustPower, sf::Color col)
    : RocketPart(relPos, col), thrust(thrustPower)
{
    // Create engine shape (a simple triangle)
    // Create engine shape (a simple triangle)
    shape.setPointCount(3);
    shape.setPoint(0, { 0, -GameConstants::ROCKET_SIZE * 2 / 3 });
    shape.setPoint(1, { -GameConstants::ROCKET_SIZE / 3, GameConstants::ROCKET_SIZE * 2 / 3 });
    shape.setPoint(2, { GameConstants::ROCKET_SIZE / 3, GameConstants::ROCKET_SIZE * 2 / 3 });
    shape.setFillColor(color);
    shape.setOrigin({ 0, 0 });
}

void Engine::draw(sf::RenderWindow& window, sf::Vector2f rocketPos, float rotation, float scale)
{
    // Scale the shape based on the zoom level
    sf::ConvexShape scaledShape = shape;
    for (size_t i = 0; i < scaledShape.getPointCount(); i++) {
        sf::Vector2f point = shape.getPoint(i);
        scaledShape.setPoint(i, point * scale);
    }

    // Calculate actual position based on rocket position and rotation
    float radians = rotation * 3.14159f / 180.0f;
    float cosA = std::cos(radians);
    float sinA = std::sin(radians);

    // Rotate the relative position
    sf::Vector2f rotatedRelPos(
        relativePosition.x * cosA - relativePosition.y * sinA,
        relativePosition.x * sinA + relativePosition.y * cosA
    );

    scaledShape.setPosition(rocketPos + rotatedRelPos * scale);
    scaledShape.setRotation(sf::degrees(rotation));
    window.draw(scaledShape);
}

float Engine::getThrust() const
{
    return thrust;
}