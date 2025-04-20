#include "Rocket.h"
#include <cmath>

Rocket::Rocket(sf::Vector2f pos, sf::Vector2f vel, sf::Color col)
    : GameObject(pos, vel, col), rotation(0), angularVelocity(0)
{
    // Create rocket body (a simple triangle)
    body.setPointCount(3);
    body.setPoint(0, { 0, -20 });
    body.setPoint(1, { -10, 20 });
    body.setPoint(2, { 10, 20 });
    body.setFillColor(color);
    body.setOrigin({ 0, 0 });
    body.setPosition(position);

    // Add default engine
    addPart(std::make_unique<Engine>(sf::Vector2f(0, 15), 100.0f));
}

void Rocket::addPart(std::unique_ptr<RocketPart> part)
{
    parts.push_back(std::move(part));
}

void Rocket::applyThrust(float amount)
{
    // Calculate thrust direction based on rocket rotation
    float radians = rotation * 3.14159f / 180.0f;
    sf::Vector2f thrustDir(-std::sin(radians), -std::cos(radians));
    velocity += thrustDir * amount;
}

void Rocket::rotate(float amount)
{
    angularVelocity += amount;
}

void Rocket::update(float deltaTime)
{
    // Update position based on velocity
    position += velocity * deltaTime;

    // Update rotation based on angular velocity
    rotation += angularVelocity * deltaTime;

    // Apply some damping to angular velocity
    angularVelocity *= 0.98f;

    // Update body position and rotation
    body.setPosition(position);
    body.setRotation(sf::degrees(rotation));
}

void Rocket::draw(sf::RenderWindow& window)
{
    // Draw rocket body
    window.draw(body);

    // Draw all rocket parts
    for (const auto& part : parts) {
        part->draw(window, position, rotation);
    }
}

void Rocket::drawVelocityVector(sf::RenderWindow& window, float scale)
{
    sf::VertexArray line(sf::PrimitiveType::LineStrip);

    sf::Vertex startVertex;
    startVertex.position = position;
    startVertex.color = sf::Color::Yellow;
    line.append(startVertex);

    sf::Vertex endVertex;
    endVertex.position = position + velocity * scale;
    endVertex.color = sf::Color::Red;
    line.append(endVertex);

    window.draw(line);
}